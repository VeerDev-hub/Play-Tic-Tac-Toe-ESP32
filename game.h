#pragma once
// Tic-tac-toe rules and the lobby. No Arduino code in here, so it can be
// compiled and tested on a normal computer.
//
// The ESP32 is the referee: phones only ask, this class decides. A device is
// identified by the IP address the ESP32 gave it, so reloading the page or
// switching browser keeps their place.
//
// Everyone with the page open is either a guest or, after picking a character,
// a player. No two players share a character, so a player's id is simply the
// number of their character. A match starts once two players have asked for
// each other; accepting an invitation is just asking back.
//
// A chat message goes either to the opponent only or to everyone. The page hides
// a message 15 seconds after showing it. Here it is erased as soon as everyone it
// was sent to has seen it.

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

class TicTacToe {
 public:
  // Keep these three in sync with AVATARS, TAUNTS and DARES in page.h.
  static const int AVATAR_COUNT = 8;
  static const int TAUNT_COUNT = 8;
  static const int DARE_COUNT = 10;

  static const int MATCH_COUNT = AVATAR_COUNT / 2;  // everybody can play at once
  static const int GUEST_COUNT = 10;                // the most an ESP32 hotspot can hold
  static const uint32_t AWAY_MS = 5000;             // shown as "wandered off"
  static const uint32_t TIMEOUT_MS = 20000;         // character and game are given up
  static const uint32_t TAUNT_COOLDOWN_MS = 1500;
  static const int NAME_BYTES = 36;             // same as NAME_BYTES in page.h
  static const int MESSAGE_COUNT = 12;          // the oldest goes when this is full
  static const int MESSAGE_BYTES = 160;         // same as MESSAGE_BYTES in page.h
  static const uint32_t MESSAGE_MS = 60000;     // erased by then, seen or not
  static const uint32_t CHAT_COOLDOWN_MS = 1000;
  static const int DRAW = 2;

  // Call first on every request: records that this device is alive and removes
  // anyone who has disappeared.
  void touch(uint32_t ip, uint32_t now) {
    int me = playerOf(ip);
    if (me >= 0) players[me].lastSeen = now;
    else touchGuest(ip, now);
    for (int i = 0; i < AVATAR_COUNT; i++) {
      if (players[i].ip && now - players[i].lastSeen > TIMEOUT_MS) clearPlayer(i);
    }
    for (int i = 0; i < GUEST_COUNT; i++) {
      if (guests[i].ip && now - guests[i].lastSeen > TIMEOUT_MS) clearGuest(i);
    }
    for (Message &m : messages) {
      if (m.waiting && now - m.sentAt > MESSAGE_MS) m = Message();
    }
  }

  // Become a player by picking a character nobody else has.
  bool join(uint32_t ip, int avatar, uint32_t now) {
    if (!ip || playerOf(ip) >= 0 || avatar < 0 || avatar >= AVATAR_COUNT) return false;
    if (players[avatar].ip) return false;
    for (Message &m : messages) {  // not from them, but from whoever had it before
      if (m.waiting && m.from == avatar) m = Message();
    }
    players[avatar] = Player();
    players[avatar].ip = ip;
    players[avatar].lastSeen = now;
    for (int i = 0; i < GUEST_COUNT; i++) {
      if (guests[i].ip != ip) continue;
      for (Message &m : messages) {  // unseen messages come along
        if (m.waiting & guestBit(i)) m.waiting |= playerBit(avatar);
      }
      clearGuest(i);
    }
    return true;
  }

  // Names are optional; without one a player goes by their character.
  bool setName(uint32_t ip, const char *name) {
    int me = playerOf(ip);
    size_t length;
    if (me < 0 || !(name = clean(name, NAME_BYTES, length))) return false;
    char text[NAME_BYTES + 1] = {};
    memcpy(text, name, length);
    for (int i = 0; i < AVATAR_COUNT; i++) {
      if (i != me && players[i].ip && !strcasecmp(players[i].name, text)) return false;
    }
    memcpy(players[me].name, text, sizeof text);
    return true;
  }

  // Give the character back and become a guest again.
  bool leave(uint32_t ip, uint32_t now) {
    int me = playerOf(ip);
    if (me < 0) return false;
    clearPlayer(me);
    touchGuest(ip, now);
    return true;
  }

  // Ask somebody to play. If they already asked you, the match starts.
  // Everyone can ask one person at a time.
  bool invite(uint32_t ip, int other) {
    int me = playerOf(ip);
    if (me < 0 || other < 0 || other >= AVATAR_COUNT || other == me) return false;
    if (!players[other].ip || players[me].match >= 0 || players[other].match >= 0) return false;
    if (players[other].ask == me) return startMatch(other, me);
    players[me].ask = other;
    players[me].refusedBy = -1;
    return true;
  }

  bool cancelInvite(uint32_t ip) {
    int me = playerOf(ip);
    if (me < 0 || players[me].ask < 0) return false;
    players[me].ask = -1;
    return true;
  }

  bool decline(uint32_t ip, int other) {
    int me = playerOf(ip);
    if (me < 0 || other < 0 || other >= AVATAR_COUNT || players[other].ask != me) return false;
    players[other].ask = -1;
    players[other].refusedBy = me;
    return true;
  }

  bool move(uint32_t ip, int cell, uint32_t randomValue) {
    int seat;
    Match *m = matchOf(ip, seat);
    if (!m || m->over || seat != m->turn) return false;
    if (cell < 0 || cell > 8 || m->board[cell] != EMPTY) return false;
    m->board[cell] = seat;

    int mask = winningCells(*m, seat);
    if (mask) {
      m->winner = seat;
      m->winMask = mask;
      m->seats[seat].score++;
      m->dare = randomValue % DARE_COUNT;
      m->over = true;
    } else if (boardFull(*m)) {
      m->winner = DRAW;
      m->over = true;
    } else {
      m->turn = 1 - seat;
    }
    return true;
  }

  bool rematch(uint32_t ip) {
    int seat;
    Match *m = matchOf(ip, seat);
    if (!m || !m->over) return false;
    m->seats[seat].again = true;
    if (m->seats[0].again && m->seats[1].again) {
      // The loser starts. After a draw, whoever did not start last time.
      startRound(*m, m->winner == DRAW ? 1 - m->starter : 1 - m->winner);
    }
    return true;
  }

  bool taunt(uint32_t ip, int id, uint32_t now) {
    int seat;
    Match *m = matchOf(ip, seat);
    if (!m || id < 0 || id >= TAUNT_COUNT) return false;
    Seat &s = m->seats[seat];
    if (s.tauntSeq && now - s.tauntAt < TAUNT_COOLDOWN_MS) return false;
    s.taunt = id;
    s.tauntAt = now;
    s.tauntSeq++;
    return true;
  }

  // Both players go back to the lobby and keep their characters.
  bool quit(uint32_t ip) {
    int me = playerOf(ip);
    if (me < 0 || players[me].match < 0) return false;
    endMatch(players[me].match);
    return true;
  }

  // Send a chat message to the opponent only, or to everyone who is here.
  bool say(uint32_t ip, bool opponentOnly, const char *text, uint32_t now) {
    int me = playerOf(ip);
    if (me < 0) return false;
    Player &p = players[me];
    if (p.hasChatted && now - p.chattedAt < CHAT_COOLDOWN_MS) return false;

    size_t length;
    if (!(text = clean(text, MESSAGE_BYTES, length))) return false;
    uint32_t others = 0;
    if (opponentOnly) {
      if (p.match < 0) return false;
      const Match &m = matches[p.match];
      others = playerBit(m.seats[m.seats[0].player == me ? 1 : 0].player);
    } else {
      for (int i = 0; i < AVATAR_COUNT; i++) {
        if (players[i].ip && i != me) others |= playerBit(i);
      }
      for (int i = 0; i < GUEST_COUNT; i++) {
        if (guests[i].ip) others |= guestBit(i);
      }
    }
    if (!others) return false;

    Message *slot = &messages[0];
    for (Message &m : messages) {
      if (!m.waiting) {
        slot = &m;
        break;
      }
      if (m.seq < slot->seq) slot = &m;
    }
    *slot = Message();
    slot->seq = ++lastMessageSeq;
    slot->sentAt = now;
    slot->from = me;
    slot->opponentOnly = opponentOnly;
    slot->waiting = others | playerBit(me);  // the sender gets it back as well
    memcpy(slot->text, text, length);
    p.hasChatted = true;
    p.chattedAt = now;
    return true;
  }

  // This device has shown these messages: a comma-separated list of numbers.
  void seen(uint32_t ip, const char *list) {
    uint32_t bit = bitOf(ip);
    if (!bit) return;
    while (*list) {
      char *end;
      uint32_t seq = strtoul(list, &end, 10);
      if (end == list) return;
      for (Message &m : messages) {
        if (m.seq != seq || !(m.waiting & bit)) continue;
        m.waiting &= ~bit;
        if (!m.waiting) m = Message();
      }
      list = *end == ',' ? end + 1 : end;
    }
  }

  // Writes the state as seen by this device, including the match it is in or,
  // failing that, the match it wants to watch. Returns the length written.
  int toJson(uint32_t ip, uint32_t watchId, uint32_t now, char *out, size_t size) const {
    int me = playerOf(ip);
    int joined = 0, here = 0;
    for (const Player &p : players) joined += p.ip != 0;
    // A guest who closed the page is not "here", even before they time out.
    for (const Guest &g : guests) here += g.ip && now - g.lastSeen <= AWAY_MS;
    here += joined;

    Writer w(out, size);
    w.add("{\"you\":%d,\"here\":%d,\"joined\":%d,\"refusedBy\":%d,\"players\":[", me, here, joined,
          me < 0 ? -1 : players[me].refusedBy);
    const char *comma = "";
    for (int i = 0; i < AVATAR_COUNT; i++) {
      const Player &p = players[i];
      if (!p.ip) continue;
      w.add("%s{\"av\":%d,\"away\":%s,\"ask\":%d,\"match\":%lu,\"name\":", comma, i, away(p, now),
            p.ask, p.match < 0 ? 0UL : (unsigned long)matches[p.match].id);
      w.addString(p.name);
      w.add("}");
      comma = ",";
    }
    w.add("],\"match\":");

    const Match *m = nullptr;
    int seat = -1;
    if (me >= 0 && players[me].match >= 0) {
      m = &matches[players[me].match];
      seat = m->seats[0].player == me ? 0 : 1;
    } else if (watchId) {
      for (const Match &candidate : matches) {
        if (candidate.id == watchId) m = &candidate;
      }
    }
    if (m) matchJson(*m, seat, now, w);
    else w.add("null");
    chatJson(ip, w);
    w.add("}");
    return w.length;
  }

 private:
  static const int8_t EMPTY = -1;

  struct Guest {
    uint32_t ip = 0;  // 0 means unused
    uint32_t lastSeen = 0;
  };
  struct Player {
    uint32_t ip = 0;  // 0 means nobody has this character
    uint32_t lastSeen = 0;
    int ask = -1;        // player they want to play
    int refusedBy = -1;  // player who last said no to them
    int match = -1;      // index in matches
    bool hasChatted = false;
    uint32_t chattedAt = 0;
    char name[NAME_BYTES + 1] = {};  // empty if they did not give one
  };
  struct Seat {
    int player = 0;
    int score = 0;
    bool again = false;
    uint32_t tauntSeq = 0;  // counts up, so phones can spot a new taunt
    uint32_t tauntAt = 0;
    int taunt = 0;
  };
  struct Match {
    uint32_t id = 0;  // 0 means unused; ids are never reused
    Seat seats[2];
    int8_t board[9] = {};
    bool over = false;
    uint32_t round = 0;
    int turn = 0, starter = 0, winner = -1, winMask = 0, dare = 0;
  };

  struct Message {
    uint32_t seq = 0;
    uint32_t sentAt = 0;
    uint32_t waiting = 0;  // who still has to see it, one bit each; 0 means unused
    int from = 0;          // player who sent it
    bool opponentOnly = false;
    char text[MESSAGE_BYTES + 1] = {};
  };

  // Appends formatted text, and stops quietly when the buffer is full.
  struct Writer {
    char *out;
    size_t size, length;
    Writer(char *buffer, size_t bufferSize) : out(buffer), size(bufferSize), length(0) {}
    void add(const char *format, ...) {
      va_list args;
      va_start(args, format);
      int n = vsnprintf(out + length, size - length, format, args);
      va_end(args);
      if (n > 0) length = length + n < size ? length + n : size - 1;
    }
    void addString(const char *text) {  // with quotes, escaped for JSON
      add("\"");
      for (; *text; text++) add(*text == '"' || *text == '\\' ? "\\%c" : "%c", *text);
      add("\"");
    }
  };

  Player players[AVATAR_COUNT];
  Guest guests[GUEST_COUNT];
  Match matches[MATCH_COUNT];
  Message messages[MESSAGE_COUNT];
  uint32_t lastMatchId = 0;
  uint32_t lastMessageSeq = 0;

  int playerOf(uint32_t ip) const {
    if (!ip) return -1;
    for (int i = 0; i < AVATAR_COUNT; i++) {
      if (players[i].ip == ip) return i;
    }
    return -1;
  }

  Match *matchOf(uint32_t ip, int &seat) {
    int me = playerOf(ip);
    if (me < 0 || players[me].match < 0) return nullptr;
    Match &m = matches[players[me].match];
    seat = m.seats[0].player == me ? 0 : 1;
    return &m;
  }

  void touchGuest(uint32_t ip, uint32_t now) {
    if (!ip) return;
    int slot = -1;
    for (int i = 0; i < GUEST_COUNT; i++) {
      const Guest &g = guests[i];
      if (g.ip == ip) {
        slot = i;
        break;
      }
      // Otherwise an unused slot, or the one that has been quiet the longest.
      if (slot < 0 || (guests[slot].ip && (!g.ip || now - g.lastSeen > now - guests[slot].lastSeen))) slot = i;
    }
    if (guests[slot].ip != ip) clearGuest(slot);
    guests[slot].ip = ip;
    guests[slot].lastSeen = now;
  }

  void clearGuest(int index) {
    forgetMessages(guestBit(index));
    guests[index] = Guest();
  }

  bool startMatch(int first, int second) {
    for (int i = 0; i < MATCH_COUNT; i++) {
      if (matches[i].id) continue;
      Match &m = matches[i];
      m = Match();
      m.id = ++lastMatchId;
      m.seats[0].player = first;
      m.seats[1].player = second;
      forgetInvites(first);
      forgetInvites(second);
      players[first].match = players[second].match = i;
      startRound(m, 0);  // whoever asked goes first
      return true;
    }
    return false;
  }

  void startRound(Match &m, int starter) {
    memset(m.board, EMPTY, sizeof m.board);
    m.starter = m.turn = starter;
    m.winner = -1;
    m.winMask = 0;
    m.seats[0].again = m.seats[1].again = false;
    m.over = false;
    m.round++;
  }

  void endMatch(int index) {
    for (Player &p : players) {
      if (p.match == index) p.match = -1;
    }
    matches[index] = Match();
  }

  // Drops everything this player asked and everything asked of them.
  void forgetInvites(int id) {
    players[id].ask = players[id].refusedBy = -1;
    for (Player &p : players) {
      if (p.ask == id) p.ask = -1;
      if (p.refusedBy == id) p.refusedBy = -1;
    }
  }

  void clearPlayer(int id) {
    if (players[id].match >= 0) endMatch(players[id].match);
    forgetInvites(id);
    forgetMessages(playerBit(id));
    players[id] = Player();
  }

  // Skips spaces around the text and returns its start, or nullptr if what is
  // left is empty, too long or has control characters in it.
  static const char *clean(const char *text, size_t maxBytes, size_t &length) {
    while (*text == ' ') text++;
    length = strlen(text);
    while (length && text[length - 1] == ' ') length--;
    if (!length || length > maxBytes) return nullptr;
    for (size_t i = 0; i < length; i++) {
      if ((unsigned char)text[i] < ' ' || text[i] == 127) return nullptr;
    }
    return text;
  }

  // ---- chat ----
  static uint32_t playerBit(int id) { return 1UL << id; }
  static uint32_t guestBit(int index) { return 1UL << (AVATAR_COUNT + index); }

  uint32_t bitOf(uint32_t ip) const {
    int me = playerOf(ip);
    if (me >= 0) return playerBit(me);
    for (int i = 0; ip && i < GUEST_COUNT; i++) {
      if (guests[i].ip == ip) return guestBit(i);
    }
    return 0;
  }

  // Nobody has to wait for this person to see anything any more.
  void forgetMessages(uint32_t bit) {
    for (Message &m : messages) {
      if (!(m.waiting & bit)) continue;
      m.waiting &= ~bit;
      if (!m.waiting) m = Message();
    }
  }

  void chatJson(uint32_t ip, Writer &w) const {
    int me = playerOf(ip);
    uint32_t bit = bitOf(ip);
    w.add(",\"unseen\":[");
    const char *comma = "";
    for (const Message &m : messages) {  // my messages that others still have to see
      if (me < 0 || m.from != me || !(m.waiting & ~bit)) continue;
      w.add("%s%lu", comma, (unsigned long)m.seq);
      comma = ",";
    }
    w.add("],\"chat\":[");
    uint32_t last = 0;
    for (int n = 0; n < 4; n++) {  // oldest first; the rest follow in the next reply
      const Message *next = nullptr;
      for (const Message &m : messages) {
        if ((m.waiting & bit) && m.seq > last && (!next || m.seq < next->seq)) next = &m;
      }
      if (!next) break;
      last = next->seq;
      w.add("%s{\"seq\":%lu,\"av\":%d,\"game\":%s,\"text\":", n ? "," : "", (unsigned long)next->seq,
            next->from, next->opponentOnly ? "true" : "false");
      w.addString(next->text);
      w.add("}");
    }
    w.add("]");
  }

  static bool boardFull(const Match &m) {
    for (int i = 0; i < 9; i++) {
      if (m.board[i] == EMPTY) return false;
    }
    return true;
  }

  // Bit i is set when cell i is part of a winning line (one move can win two).
  static int winningCells(const Match &m, int seat) {
    static const uint8_t LINES[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6},
                                        {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};
    int mask = 0;
    for (const auto &l : LINES) {
      if (m.board[l[0]] == seat && m.board[l[1]] == seat && m.board[l[2]] == seat) {
        mask |= (1 << l[0]) | (1 << l[1]) | (1 << l[2]);
      }
    }
    return mask;
  }

  static const char *away(const Player &p, uint32_t now) {
    return now - p.lastSeen > AWAY_MS ? "true" : "false";
  }

  void matchJson(const Match &m, int seat, uint32_t now, Writer &w) const {
    char cells[10];
    for (int i = 0; i < 9; i++) cells[i] = m.board[i] == EMPTY ? '.' : (char)('0' + m.board[i]);
    cells[9] = 0;
    w.add("{\"id\":%lu,\"seat\":%d,\"phase\":\"%s\",\"round\":%lu,\"board\":\"%s\",\"turn\":%d,"
          "\"winner\":%d,\"mask\":%d,\"dare\":%d,\"players\":[",
          (unsigned long)m.id, seat, m.over ? "over" : "play", (unsigned long)m.round, cells, m.turn,
          m.winner, m.winMask, m.dare);
    for (int i = 0; i < 2; i++) {
      const Seat &s = m.seats[i];
      w.add("%s{\"av\":%d,\"score\":%d,\"away\":%s,\"again\":%s,\"tauntSeq\":%lu,\"taunt\":%d}",
            i ? "," : "", s.player, s.score, away(players[s.player], now),
            s.again ? "true" : "false", (unsigned long)s.tauntSeq, s.taunt);
    }
    w.add("]}");
  }
};
