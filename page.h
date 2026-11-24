#pragma once
// The whole game page. Everything is inline because the hotspot has no internet.
// AVATARS, TAUNTS, DARES, NAME_BYTES and MESSAGE_BYTES below must match game.h.

static const char PAGE[] PROGMEM = R"rawpage(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, interactive-widget=resizes-content">
<title>Tic-Tac-Toe</title>
<style>
  :root {
    --paper: #fff8e7; --ink: #33261d; --soft: #7a6554; --line: #e3d3b4;
    --p0: #c8321f; --p0-tint: #fbd9cf; --p1: #1c6a8c; --p1-tint: #cfe8f2;
  }
  * { box-sizing: border-box; }
  [hidden] { display: none !important; }
  body {
    margin: 0; min-height: 100vh; padding: 14px;
    font-family: ui-rounded, "Arial Rounded MT Bold", "Nunito", system-ui, -apple-system, Roboto, sans-serif;
    color: var(--ink); line-height: 1.35;
    /* picnic tablecloth */
    background-color: #fff;
    background-image:
      linear-gradient(90deg, rgba(216,65,47,.62) 50%, transparent 50%),
      linear-gradient(rgba(216,65,47,.62) 50%, transparent 50%);
    background-size: 44px 44px;
  }
  main {
    max-width: 440px; margin: 0 auto; padding: 18px 16px 16px;
    background: var(--paper); border-radius: 6px;
    box-shadow: 0 3px 0 #d9c69f, 0 10px 24px rgba(70,20,10,.35);
  }
  h1 { margin: 0 0 4px; font-size: 1.7rem; line-height: 1.1; letter-spacing: -.02em; text-wrap: balance; }
  p { margin: 0 0 14px; }
  .soft { color: var(--soft); }
  button { font: inherit; color: inherit; cursor: pointer; -webkit-tap-highlight-color: transparent; touch-action: manipulation; }
  button:focus-visible { outline: 3px solid var(--ink); outline-offset: 2px; }

  .primary {
    display: block; width: 100%; padding: 13px; border: 0; border-radius: 12px;
    background: var(--p0); color: #fff; font-size: 1.1rem; font-weight: 700;
    box-shadow: 0 4px 0 #8c1f12;
  }
  .primary:active { transform: translateY(3px); box-shadow: 0 1px 0 #8c1f12; }
  .primary:disabled { background: #cbb99b; box-shadow: 0 4px 0 #ab9a7d; cursor: default; }
  .plain { display: block; margin: 12px auto 0; padding: 8px 12px; border: 0; background: none; color: var(--soft); text-decoration: underline; }

  .picker { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; margin-bottom: 16px; }
  .picker button { padding: 8px 2px; border: 2px solid var(--line); border-radius: 12px; background: #fff; font-size: .78rem; }
  .picker span { display: block; font-size: 2rem; line-height: 1.2; }
  .picker [aria-pressed="true"] { border-color: var(--p0); background: var(--p0-tint); }
  .picker [aria-pressed="true"] span { animation: plop .25s; }
  .picker button:disabled { opacity: .35; cursor: default; }

  .versus { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
  .card { position: relative; display: flex; align-items: center; gap: 6px; padding: 8px 7px; border-radius: 12px; border: 3px solid transparent; min-width: 0; }
  .card.s0 { background: var(--p0-tint); }
  .card.s1 { background: var(--p1-tint); }
  .card.s0.active { border-color: var(--p0); }
  .card.s1.active { border-color: var(--p1); }
  .card.away { opacity: .55; }
  .card .face { font-size: 1.5rem; line-height: 1; }
  .card .who { flex: 1; min-width: 0; font-size: .84rem; font-weight: 700; overflow-wrap: anywhere; }
  .card .who small { display: block; font-weight: 400; color: var(--soft); }
  .card .score { font-size: 1.5rem; font-weight: 700; font-variant-numeric: tabular-nums; }
  .bubble {
    position: absolute; z-index: 5; inset: -3px; padding: 4px 8px; display: grid; place-items: center;
    border-radius: 12px; background: var(--ink); color: #fff; font-size: .95rem; font-weight: 700; text-align: center;
    animation: plop .25s;
  }

  #status { min-height: 1.4em; margin: 12px 0; text-align: center; font-size: 1.08rem; font-weight: 700; }
  .board { display: grid; grid-template-columns: repeat(3, 1fr); width: 100%; max-width: min(100%, 52vh); min-width: 230px; margin: 0 auto 14px; }
  .cell {
    aspect-ratio: 1; min-width: 0; padding: 0; border: 0 solid var(--ink); background: none;
    display: grid; place-items: center; font-size: clamp(2.2rem, 13vw, 3.6rem); line-height: 1; border-radius: 0; cursor: default;
  }
  /* crayon-like hash */
  .cell:nth-child(3n+1), .cell:nth-child(3n+2) { border-right-width: 5px; }
  .cell:nth-child(-n+6) { border-bottom-width: 5px; }
  .cell.s0 { background: var(--p0-tint); }
  .cell.s1 { background: var(--p1-tint); }
  .cell span { animation: plop .25s; }
  .board.mine .cell:not(.s0):not(.s1) { cursor: pointer; }
  .board.mine .cell:not(.s0):not(.s1):active { background: var(--line); }
  .cell.win span { animation: wiggle .5s infinite alternate ease-in-out; }
  .cell.win.s0 { background: #f3a493; }
  .cell.win.s1 { background: #8fcbe2; }
  .board.lost { animation: shake .5s; }

  .dare { margin: 0 0 14px; padding: 12px 14px; border: 3px dashed var(--ink); border-radius: 12px; text-align: center; }
  .dare b { display: block; font-size: 1.12rem; }
  .taunts { display: flex; flex-wrap: wrap; justify-content: center; gap: 6px; margin-top: 18px; }
  .taunts button { padding: 8px 11px; border: 2px solid var(--line); border-radius: 99px; background: #fff; font-size: .86rem; }
  .taunts button:disabled { opacity: .45; cursor: default; }

  #notice { position: fixed; z-index: 20; left: 12px; right: 12px; top: 14px; max-width: 420px; margin: 0 auto; padding: 11px 14px; border-radius: 12px; background: var(--ink); color: #fff; text-align: center; }
  .confetti { position: fixed; z-index: 10; top: -50px; font-size: 1.8rem; pointer-events: none; animation: fall 2.6s linear forwards; }

  .counts { display: flex; justify-content: center; flex-wrap: wrap; gap: 2px 16px; margin: -4px 0 14px; padding-bottom: 10px; border-bottom: 2px solid var(--line); font-size: .9rem; color: var(--soft); }
  .counts b { color: var(--ink); font-size: 1.05rem; font-variant-numeric: tabular-nums; }
  .invite { margin: 0 0 14px; padding: 12px; border: 3px solid var(--p1); border-radius: 12px; background: var(--p1-tint); font-weight: 700; }
  .invite div { display: flex; gap: 8px; margin-top: 10px; }
  .invite button { flex: 1; }
  #join:not([hidden]) + #lobby { margin-top: 22px; }
  .row { display: flex; align-items: center; gap: 10px; min-height: 62px; padding: 8px 0; border-top: 2px solid var(--line); }
  .row.away .face, .row.away .who { opacity: .6; }
  .row .face { font-size: 1.8rem; line-height: 1; }
  .row .who { flex: 1; min-width: 0; font-weight: 700; overflow-wrap: anywhere; }
  .row .who small { display: block; font-weight: 400; color: var(--soft); }
  .small { min-height: 44px; padding: 8px 14px; border: 2px solid var(--line); border-radius: 12px; background: #fff; font-weight: 700; }
  .small.go { border-color: #8c1f12; background: var(--p0); color: #fff; }

  /* Chat floats above the page, so messages coming and going never move the board. */
  body.chatting { padding-bottom: 92px; }
  body.chatting .board { max-width: min(100%, 42vh); min-width: 200px; }   /* leaves room for messages */
  body.chatting.seated { padding-bottom: 138px; }
  body.chatting.seated .board { max-width: min(100%, 38vh); }
  @media (max-height: 640px) { body.seated .counts { display: none; } }
  #chat { position: fixed; z-index: 8; left: 0; right: 0; bottom: 0; pointer-events: none; }
  #chat > * { max-width: 440px; margin: 0 auto; }
  #messages { display: flex; flex-direction: column; gap: 6px; max-height: 30vh; padding: 0 14px 8px; overflow-y: auto; }
  #messages > :first-child { margin-top: auto; }
  .msg {
    position: relative; flex: none; align-self: flex-start; max-width: 86%; padding: 7px 12px 9px; overflow: hidden; pointer-events: auto;
    border: 2px solid var(--ink); border-radius: 14px 14px 14px 4px; background: #fff; box-shadow: 0 3px 8px rgba(70,20,10,.3);
    font-size: .95rem; overflow-wrap: anywhere; animation: rise .2s;
  }
  .msg.own { align-self: flex-end; border-radius: 14px 14px 4px 14px; background: #ffe9a8; }
  .msg b { margin-right: 6px; }
  .msg small { margin-left: 8px; color: var(--soft); }
  .msg i { position: absolute; left: 0; bottom: 0; width: 100%; height: 4px; background: var(--p0); transform-origin: left; animation: burn 15s linear forwards; }
  #nameBox { display: block; margin-bottom: 16px; font-weight: 700; }
  #nameBox input { display: block; width: 100%; margin-top: 6px; }
  .field { padding: 10px 12px; border: 2px solid var(--line); border-radius: 12px; background: #fff; color: inherit; font: inherit; font-size: 16px; font-weight: 400; }
  .field:focus-visible { outline: 3px solid var(--ink); outline-offset: 1px; }
  .field::placeholder { color: var(--soft); opacity: 1; }
  .msg em { margin-left: 8px; font-size: .78rem; font-style: normal; color: var(--soft); }
  .msg.game { background: var(--p1-tint); }
  .msg.own.game { background: #ffd9a0; }
  #sayTo { display: flex; gap: 8px; padding: 8px 14px 0; background: var(--paper); border-top: 3px solid var(--ink); pointer-events: auto; }
  #sayTo button { flex: 1; min-width: 0; min-height: 38px; padding: 4px 8px; border: 2px solid var(--line); border-radius: 99px; background: #fff; font-size: .88rem; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; }
  #sayTo [aria-pressed="true"] { border-color: var(--ink); background: var(--ink); color: #fff; font-weight: 700; }
  #sayTo:not([hidden]) + #sayForm { border-top: 0; padding-top: 8px; }
  #sayForm { display: flex; gap: 8px; padding: 10px 14px; background: var(--paper); border-top: 3px solid var(--ink); pointer-events: auto; }
  #sayText { flex: 1; min-width: 0; }
  #sayForm .small:disabled { border-color: var(--line); background: #efe4cb; color: var(--soft); cursor: default; }
  #sayForm .small { min-height: 46px; }

  @keyframes rise { from { transform: translateY(14px); opacity: 0; } }
  @keyframes burn { to { transform: scaleX(0); } }
  @keyframes plop { from { transform: scale(.3); } 70% { transform: scale(1.15); } to { transform: scale(1); } }
  @keyframes wiggle { from { transform: rotate(-14deg); } to { transform: rotate(14deg) scale(1.12); } }
  @keyframes shake { 20%, 60% { transform: translateX(-7px); } 40%, 80% { transform: translateX(7px); } }
  @keyframes fall { to { transform: translateY(115vh) rotate(540deg); } }
  @media (prefers-reduced-motion: reduce) { * { animation: none !important; } .confetti, .msg i { display: none; } }
</style>
</head>
<body>
<main>
  <div id="counts" class="counts" hidden>
    <span><b id="hereCount"></b> here</span>
    <span><b id="joinedCount"></b> joined</span>
    <span><b id="playingCount"></b> playing</span>
  </div>
  <div id="invites" aria-live="polite"></div>

  <section id="join" hidden>
    <h1>Pick your fighter</h1>
    <p class="soft">Then choose who to play. The loser has to do a dare.</p>
    <div id="picker" class="picker"></div>
    <form id="joinForm" autocomplete="off">
      <label id="nameBox" hidden>Your name
        <input id="nameText" class="field" type="text" enterkeyhint="go" autocomplete="off" spellcheck="false">
      </label>
      <button id="joinBtn" class="primary" disabled>Pick one to join</button>
    </form>
  </section>

  <section id="lobby" hidden>
    <h1 id="lobbyTitle"></h1>
    <p id="lobbyText" class="soft"></p>
    <div id="rows"></div>
    <button id="leaveBtn" class="plain">Change character</button>
  </section>

  <section id="game" hidden>
    <div class="versus">
      <div class="card s0" id="card0"></div>
      <div class="card s1" id="card1"></div>
    </div>
    <div id="status" role="status"></div>
    <div id="board" class="board"></div>
    <div id="dare" class="dare" hidden></div>
    <button id="againBtn" class="primary" hidden>Rematch</button>
    <div id="taunts" class="taunts"></div>
    <button id="quitBtn" class="plain"></button>
  </section>

  <p id="loading" class="soft" style="margin:0;text-align:center">Finding the game&hellip;</p>
</main>
<div id="notice" role="alert" hidden></div>
<div id="chat">
  <div id="messages" role="log" aria-label="Chat"></div>
  <div id="sayTo" role="group" aria-label="Who to message" hidden>
    <button id="toGame"></button>
    <button id="toAll">Everyone</button>
  </div>
  <form id="sayForm" autocomplete="off" hidden>
    <input id="sayText" class="field" type="text" aria-label="Chat message" enterkeyhint="send" autocomplete="off">
    <button id="sayBtn" class="small go">Send</button>
  </form>
</div>

<script>
(function () {
  var AVATARS = [
    ['\u{1F414}', 'Chicken'], ['\u{1F954}', 'Potato'], ['\u{1F438}', 'Frog'], ['\u{1F355}', 'Pizza'],
    ['\u{1F996}', 'Dino'], ['\u{1F921}', 'Clown'], ['\u{1F9E6}', 'Sock'], ['\u{1F951}', 'Avocado']
  ];
  var TAUNTS = [
    'Was that on purpose?', 'My grandma is faster', 'I smell fear', 'Tick tock...',
    'Beep boop, you lose', 'Nooooo!', 'Ha ha ha!', 'OK, respect.'
  ];
  var DARES = [   // {w} is the winner's character
    'Cluck like a chicken for 10 seconds.',
    'Talk like a robot until the next game ends.',
    'Compliment the winner\'s shoes. Sincerely.',
    'Do five jumping jacks in slow motion.',
    'Sing the next thing you say.',
    'Announce proudly: "I lost to a {w}."',
    'Walk like a penguin to the nearest wall and back.',
    'Give a 10-second acceptance speech for losing.',
    'Use your hand as a phone and order a pizza.',
    'Find a wall and apologise to it.'
  ];
  var NAME_CHARS = 12, NAME_BYTES = 36;          // longest name
  var MESSAGE_BYTES = 160, MESSAGE_MS = 15000;   // longest message; how long it stays once seen
  var THINKING = [
    '{o} is thinking. Allegedly.', '{o} is planning something silly.',
    'Waiting for {o}. Any day now.', '{o} is staring at the board.'
  ];

  var $ = function (id) { return document.getElementById(id); };
  var state = null, match = null, lastText = '', picked = -1;
  var watching = 0, quitting = 0, shown = 0;      // match ids
  var sent = 0, applied = 0, failures = 0, pollTimer, noticeTimer;
  var seenTaunt = [null, null], bubbles = [null, null], celebrated = null;
  var rows = [], invites = [];
  var messages = {}, sending = false;             // chat messages this phone has, by key
  var toGame = true, draftMatch = 0;              // who the chat box writes to

  // What somebody is called: their own name if they gave one, or their character.
  function nameOf(av, from) {
    var p = (from || state).players.filter(function (p) { return p.av === av; })[0];
    return p && p.name || AVATARS[av][1];
  }
  function name(seat) { return nameOf(match.players[seat].av); }
  function face(seat) { return AVATARS[match.players[seat].av][0]; }
  function player(av) {
    return state.players.filter(function (p) { return p.av === av; })[0] || null;
  }

  function el(tag, cls, text) {
    var e = document.createElement(tag);
    if (cls) e.className = cls;
    if (text !== undefined) e.textContent = text;
    return e;
  }
  function setText(e, text) { if (e.textContent !== text) e.textContent = text; }

  // ---- talking to the ESP32 ----
  function call(method, path) {
    var id = ++sent, watched = watching;
    var abort = new AbortController();
    var timer = setTimeout(function () { abort.abort(); }, 4000);
    if (watched) path += (path.indexOf('?') < 0 ? '?' : '&') + 'watch=' + watched;
    var acks = seenMessages();
    if (acks) path += (path.indexOf('?') < 0 ? '?' : '&') + 'seen=' + acks;
    return fetch('/api/' + path, { method: method, cache: 'no-store', signal: abort.signal })
      .then(function (r) { if (!r.ok) throw new Error(r.status); return r.text(); })
      .then(function (text) {
        clearTimeout(timer);
        if (failures >= 3) $('notice').hidden = true;
        failures = 0;
        // Skip an older reply that arrived late, or one about another game.
        if (id < applied || watched !== watching) return;
        var next = JSON.parse(text);
        applied = id;
        if (text !== lastText) { lastText = text; update(next); }
      })
      .catch(function (err) {
        clearTimeout(timer);
        if (++failures >= 3) notice('Lost the ESP32. Stay on this WiFi, still trying...', true);
        throw err;
      });
  }
  function act(path) {
    return call('POST', path).then(function () { return true; }, function () {
      if (failures < 3) notice('That did not go through. Try again.');
      return false;
    });
  }
  function poll() {
    clearTimeout(pollTimer);
    call('GET', 'state').catch(function () {}).then(function () {
      clearTimeout(pollTimer);
      pollTimer = setTimeout(poll, 700);
    });
  }
  function notice(text, stay) {
    clearTimeout(noticeTimer);
    $('notice').textContent = text;
    $('notice').hidden = false;
    if (!stay) noticeTimer = setTimeout(function () { $('notice').hidden = true; }, 3000);
  }
  function watch(id) {
    watching = id;
    lastText = '';      // draw the next reply even if nothing else changed
    poll();
  }

  // ---- drawing ----
  function update(next) {
    var before = match, old = state;
    state = next;
    match = next.match;

    if ((match ? match.id : 0) !== shown) {      // a different game, or none
      shown = match ? match.id : 0;
      seenTaunt = [null, null];
      bubbles = [null, null];
      celebrated = null;
      toGame = true;
    }
    if (before && (!match || match.id !== before.id)) {
      if (before.seat >= 0) {
        if (state.you < 0) notice('You were gone too long, so your game ended.');
        else if (quitting !== before.id) notice(nameOf(before.players[1 - before.seat].av, old) + ' left the game.');
      } else if (watching && !match) notice('That game is over.');
    }
    if (!match || match.seat >= 0) watching = 0;
    receiveChat();
    render();
  }

  function render() {
    var joining = !match && state.you < 0;
    $('loading').hidden = true;
    $('counts').hidden = false;
    setText($('hereCount'), String(state.here));
    setText($('joinedCount'), String(state.joined));
    setText($('playingCount'), String(state.players.filter(function (p) { return p.match; }).length));
    $('join').hidden = !joining;
    // Guests see who has joined too, below the character picker.
    $('lobby').hidden = !!match || (joining && !state.players.length);
    $('game').hidden = !match;

    var free = state.you >= 0 && !(match && match.seat >= 0);
    invites.forEach(function (card, av) {
      var p = player(av);
      card.hidden = !(free && p && p.ask === state.you);
      setText(card.firstChild, AVATARS[av][0] + ' ' + nameOf(av) + ' wants to play you!');
    });
    if (joining) renderJoin();
    if (match) { checkTaunts(); renderGame(); } else renderLobby();
    renderChat();
  }

  function renderJoin() {
    if (picked >= 0 && player(picked)) picked = -1;
    Array.prototype.forEach.call($('picker').children, function (b, i) {
      b.disabled = !!player(i);
      b.setAttribute('aria-pressed', String(i === picked));
    });
    $('joinBtn').disabled = picked < 0;
    $('nameBox').hidden = picked < 0;
    if (picked < 0) return setText($('joinBtn'), 'Pick one to join');
    $('nameText').placeholder = 'Or just be ' + AVATARS[picked][1];
    setText($('joinBtn'), 'Join as ' + (tidy($('nameText').value) || AVATARS[picked][1]));
  }

  // What the button in somebody's row does right now.
  function rowAction(p) {
    var me = player(state.you);
    if (p.match) return 'watch';
    if (!me || p === me || p.ask === me.av) return '';   // an invitation has its own buttons
    return me.ask === p.av ? 'cancel' : 'ask';
  }

  function renderLobby() {
    var me = player(state.you);
    setText($('lobbyTitle'), me ? 'Who do you want to play?' : 'Already joined');
    setText($('lobbyText'), !me ? 'Pick a fighter above to play one of them.'
      : state.players.length < 2 ? 'Nobody else has joined yet. Get a friend on this WiFi!'
      : 'Ask someone. The game starts when they say yes.');
    $('leaveBtn').hidden = !me;
    rows.forEach(function (r, av) {
      var p = player(av);
      r.row.hidden = !p;
      if (!p) return;
      var rival = p.match && state.players.filter(function (q) {
        return q !== p && q.match === p.match;
      })[0];
      var text = rival ? 'playing ' + nameOf(rival.av) : 'ready to play';
      if (p === me) text = 'you';
      else if (!p.match) {
        if (me && p.ask === me.av) text = 'wants to play you!';
        else if (p.away) text = 'wandered off';
        else if (me && me.ask === av) text = 'thinking about it...';
        else if (me && state.refusedBy === av) text = 'said no thanks';
      }
      var action = rowAction(p);
      r.row.className = 'row' + (p.away ? ' away' : '');
      setText(r.name, nameOf(av));
      setText(r.note, text);
      r.btn.hidden = !action;
      r.btn.className = 'small' + (action === 'ask' ? ' go' : '');
      setText(r.btn, action === 'watch' ? 'Watch' : action === 'cancel' ? 'Cancel'
        : me && state.refusedBy === av ? 'Ask again' : 'Ask to play');
      r.btn.setAttribute('aria-label', r.btn.textContent + ': ' + nameOf(av));
    });
  }

  function renderGame() {
    var me = match.seat, opp = 1 - me, seated = me >= 0;
    var over = match.phase === 'over', playing = !over;

    [0, 1].forEach(function (s) {
      var card = $('card' + s), p = match.players[s];
      card.className = 'card s' + s + (playing && match.turn === s ? ' active' : '') + (p.away ? ' away' : '');
      card.textContent = '';
      card.appendChild(el('span', 'face', face(s)));
      var who = el('div', 'who', name(s));
      who.appendChild(el('small', '', s === me ? 'you' : p.away ? 'wandered off' : ''));
      card.appendChild(who);
      card.appendChild(el('span', 'score', p.score));
      if (bubbles[s]) card.appendChild(bubbles[s]);
    });

    var text;
    if (playing) {
      if (!seated) text = name(match.turn) + ' to move. You are watching.';
      else if (match.turn === me) text = 'Your move, ' + name(me) + '.';
      else if (match.players[opp].away) text = name(opp) + ' wandered off. Go find them.';
      else text = THINKING[match.round % THINKING.length].replace('{o}', function () { return name(opp); });
    } else if (match.winner === 2) text = seated ? 'A draw. You are both equally average.' : 'A draw!';
    else if (!seated) text = name(match.winner) + ' wins!';
    else text = match.winner === me ? 'You win!' : name(opp) + ' wins. Ouch.';
    setText($('status'), text);

    var board = $('board');
    board.className = 'board' + (playing && seated && match.turn === me ? ' mine' : '') +
      (over && seated && match.winner === opp ? ' lost' : '');
    Array.prototype.forEach.call(board.children, function (c, i) {
      var v = match.board[i];
      c.className = 'cell' + (v === '.' ? '' : ' s' + v) + (match.mask >> i & 1 ? ' win' : '');
      var f = v === '.' ? '' : face(+v);
      if (c.textContent !== f) {
        c.textContent = '';
        if (f) c.appendChild(el('span', '', f));
      }
      c.setAttribute('aria-label', 'Row ' + (Math.floor(i / 3) + 1) + ', column ' + (i % 3 + 1) + ': ' +
        (v === '.' ? 'empty' : name(+v)));
    });

    var hasDare = over && match.winner !== 2;
    $('dare').hidden = !hasDare;
    if (hasDare) {
      var lead = seated && match.winner === opp ? 'Your dare:' : 'Dare for ' + name(1 - match.winner) + ':';
      $('dare').textContent = lead;
      $('dare').appendChild(el('b', '', DARES[match.dare].replace('{w}', AVATARS[match.players[match.winner].av][1])));
    }

    var again = over && seated;
    $('againBtn').hidden = !again;
    if (again) {
      var asked = match.players[me].again;
      $('againBtn').disabled = asked;
      setText($('againBtn'), asked ? 'Waiting for ' + name(opp) + '...'
        : match.players[opp].again ? name(opp) + ' wants a rematch!' : 'Rematch');
    }
    $('taunts').hidden = !seated;
    setText($('quitBtn'), !seated ? 'Stop watching' : over ? 'Play someone else' : 'Leave the game');

    if (!over) celebrated = null;
    else if (celebrated !== match.round) {
      celebrated = match.round;
      if (match.winner !== 2 && (!seated || match.winner === me)) confetti(face(match.winner));
    }
  }

  function checkTaunts() {
    [0, 1].forEach(function (s) {
      var p = match.players[s];
      if (seenTaunt[s] !== null && p.tauntSeq > seenTaunt[s]) {
        var b = el('div', 'bubble', TAUNTS[p.taunt]);
        bubbles[s] = b;
        setTimeout(function () {
          if (bubbles[s] === b) bubbles[s] = null;
          b.remove();
        }, 2600);
      }
      seenTaunt[s] = p.tauntSeq;
    });
  }

  // ---- chat ----
  // A message is kept only while it is on screen or the ESP32 still offers it.
  // Showing it counts as seeing it, so nothing is shown while the page is hidden.
  function receiveChat() {
    Object.keys(messages).forEach(function (key) { messages[key].offered = false; });
    state.chat.forEach(function (m) {
      var key = m.seq + ':' + m.av + ':' + m.text;
      if (!messages[key]) messages[key] = { key: key, seq: m.seq, av: m.av, game: m.game, text: m.text, own: m.av === state.you, box: null, gone: false };
      messages[key].offered = true;
    });
    Object.keys(messages).forEach(function (key) {
      var m = messages[key];
      if (!m.offered && (m.gone || !m.box)) delete messages[key];
      // Your own message waits until everyone has seen it.
      else if (m.box && m.own && !m.timer && state.unseen.indexOf(m.seq) < 0) countDown(m);
    });
    showMessages();
  }

  function showMessages() {
    if (document.hidden) return;
    Object.keys(messages).map(function (key) { return messages[key]; })
      .filter(function (m) { return !m.box && !m.gone; })
      .sort(function (a, b) { return a.seq - b.seq; })
      .forEach(function (m) {
        m.box = el('div', 'msg' + (m.own ? ' own' : '') + (m.game ? ' game' : ''));
        m.box.appendChild(el('b', '', AVATARS[m.av][0] + (m.own ? '' : ' ' + nameOf(m.av))));
        m.box.appendChild(document.createTextNode(m.text));
        if (match && match.seat >= 0 && !m.game) m.box.appendChild(el('em', '', 'to everyone'));
        m.box.onclick = function () { removeMessage(m); };
        $('messages').appendChild(m.box);
        if (!m.own || state.unseen.indexOf(m.seq) < 0) countDown(m);
        else m.box.appendChild(el('small', '', 'unseen'));
        $('messages').scrollTop = $('messages').scrollHeight;
      });
  }

  function countDown(m) {
    var note = m.box.querySelector('small');
    if (note) note.remove();
    m.box.appendChild(el('i'));
    m.timer = setTimeout(function () { removeMessage(m); }, MESSAGE_MS);
  }

  function removeMessage(m) {
    clearTimeout(m.timer);
    if (m.box) m.box.remove();
    m.box = null;
    m.gone = true;
    if (!m.offered) delete messages[m.key];
  }

  // Messages shown here that the ESP32 does not know about yet.
  function seenMessages() {
    return Object.keys(messages).map(function (key) { return messages[key]; })
      .filter(function (m) { return m.offered && (m.box || m.gone); })
      .map(function (m) { return m.seq; }).join(',');
  }

  function seated() { return !!match && match.seat >= 0; }
  function writingToGame() { return seated() && toGame; }
  function somebodyListens() { return writingToGame() || state.here > 1; }

  function renderChat() {
    // Text written for an opponent must not end up in the global chat, or with
    // the next opponent, because the game ended halfway through a sentence.
    var here = writingToGame() ? match.id : 0;
    if (draftMatch && draftMatch !== here) $('sayText').value = '';
    draftMatch = here;

    var canChat = state.you >= 0;
    $('sayForm').hidden = !canChat;
    $('sayTo').hidden = !(canChat && seated());
    document.body.classList.toggle('chatting', canChat);
    document.body.classList.toggle('seated', seated());
    if (seated()) {
      setText($('toGame'), 'Only ' + name(1 - match.seat));
      $('toGame').setAttribute('aria-pressed', String(toGame));
      $('toAll').setAttribute('aria-pressed', String(!toGame));
    }
    var hint = writingToGame() ? 'Message ' + name(1 - match.seat)
      : somebodyListens() ? 'Message everyone' : 'Nobody here to chat with yet';
    if ($('sayText').placeholder !== hint) $('sayText').placeholder = hint;
    $('sayBtn').disabled = sending || !somebodyListens() || !$('sayText').value.trim();
  }

  function writeTo(game) {
    draftMatch = 0;      // switching on purpose keeps what you wrote
    toGame = game;
    renderChat();
    $('sayText').focus();
  }

  // Removes control characters and the invisible ones that can flip text around.
  function tidy(text) {
    return text.replace(/[\u0000-\u001f\u007f\u200b\u200e\u200f\u202a-\u202e\u2066-\u2069]/g, ' ').trim();
  }

  // Shortens what is in a field by whole characters, so an emoji is never cut in half.
  function limit(field, maxBytes, maxChars) {
    var chars = Array.from(field.value);
    while (chars.length && (chars.length > maxChars || byteLength(chars.join('')) > maxBytes)) chars.pop();
    if (chars.join('') !== field.value) field.value = chars.join('');
  }

  function byteLength(text) {
    try { return encodeURIComponent(text).replace(/%../g, 'x').length; } catch (e) { return Infinity; }
  }

  function say() {
    var text = tidy($('sayText').value), game = writingToGame(), matchId = match ? match.id : 0;
    if (!text || sending || !somebodyListens()) return;
    if (byteLength(text) > MESSAGE_BYTES) return notice('That message is too long.');
    sending = true;
    $('sayText').value = '';
    renderChat();
    act('say?game=' + (game ? 1 : 0) + '&text=' + encodeURIComponent(text)).then(function (ok) {
      // Put it back, but only if it would still go to the same people.
      var same = game ? writingToGame() && match.id === matchId : !writingToGame();
      if (!ok && same && !$('sayText').value) $('sayText').value = text;
      setTimeout(function () { sending = false; renderChat(); }, ok ? 1100 : 0);
      renderChat();
    });
  }

  function confetti(emoji) {
    for (var i = 0; i < 22; i++) {
      var c = el('span', 'confetti', emoji);
      c.style.left = Math.random() * 95 + 'vw';
      c.style.animationDelay = Math.random() * 0.9 + 's';
      document.body.appendChild(c);
      setTimeout(function (e) { e.remove(); }, 4000, c);
    }
  }

  // ---- set up the buttons once ----
  AVATARS.forEach(function (a, i) {
    var b = el('button');
    b.appendChild(el('span', '', a[0]));
    b.appendChild(document.createTextNode(a[1]));
    b.setAttribute('aria-pressed', 'false');
    b.onclick = function () { picked = i; renderJoin(); };
    $('picker').appendChild(b);

    // One lobby row and one invitation per character, shown when needed. They
    // stay in place between updates, so a tap is never lost to a redraw.
    var row = el('div', 'row'), who = el('div', 'who'), called = el('span', '', a[1]), note = el('small'), btn = el('button', 'small');
    who.appendChild(called);
    who.appendChild(note);
    row.appendChild(el('span', 'face', a[0]));
    row.appendChild(who);
    row.appendChild(btn);
    row.hidden = true;
    btn.onclick = function () {
      var p = player(i), action = p && rowAction(p);
      if (action === 'watch') watch(p.match);
      else if (action === 'cancel') act('cancel');
      else if (action === 'ask') act('invite?player=' + i);
    };
    $('rows').appendChild(row);
    rows.push({ row: row, name: called, note: note, btn: btn });

    var card = el('div', 'invite');
    card.appendChild(el('span'));
    var buttons = el('div'), yes = el('button', 'small go', 'Play'), no = el('button', 'small', 'No thanks');
    yes.onclick = function () { act('invite?player=' + i); };
    no.onclick = function () { act('decline?player=' + i); };
    buttons.appendChild(yes);
    buttons.appendChild(no);
    card.appendChild(buttons);
    card.hidden = true;
    $('invites').appendChild(card);
    invites.push(card);
  });
  for (var i = 0; i < 9; i++) (function (i) {
    var c = el('button', 'cell');
    c.onclick = function () {
      if (match && match.phase === 'play' && match.turn === match.seat && match.board[i] === '.') act('move?cell=' + i);
    };
    $('board').appendChild(c);
  })(i);
  TAUNTS.forEach(function (t, i) {
    var b = el('button', '', t);
    b.onclick = function () {
      act('taunt?id=' + i);
      var all = $('taunts').children;
      Array.prototype.forEach.call(all, function (x) { x.disabled = true; });
      setTimeout(function () { Array.prototype.forEach.call(all, function (x) { x.disabled = false; }); }, 1800);
    };
    $('taunts').appendChild(b);
  });
  $('joinForm').onsubmit = function (e) {
    e.preventDefault();
    if (picked < 0) return;
    var wanted = tidy($('nameText').value), lower = wanted.toLowerCase();
    if (wanted && state.players.some(function (p) { return nameOf(p.av).toLowerCase() === lower; })) {
      return notice('Somebody here is already called ' + wanted + '.');
    }
    try { localStorage.setItem('name', wanted); } catch (err) {}
    $('nameText').blur();
    act('join?avatar=' + picked + '&name=' + encodeURIComponent(wanted)).then(function (ok) {
      if (!ok || !state) return;
      if (state.you < 0) notice('Someone beat you to it. Pick another.');
      else if (wanted && player(state.you).name !== wanted) notice('That name was just taken, so you are ' + nameOf(state.you) + '.');
    });
  };
  $('nameText').oninput = function () { limit(this, NAME_BYTES, NAME_CHARS); renderJoin(); };
  try { $('nameText').value = localStorage.getItem('name') || ''; } catch (err) {}
  limit($('nameText'), NAME_BYTES, NAME_CHARS);
  $('toGame').onclick = function () { writeTo(true); };
  $('toAll').onclick = function () { writeTo(false); };
  $('againBtn').onclick = function () { act('rematch'); };
  $('leaveBtn').onclick = function () { picked = -1; act('leave'); };
  $('quitBtn').onclick = function () {
    if (!match) return;
    if (match.seat >= 0) { quitting = match.id; act('quit'); }
    else {
      match = null;
      watch(0);
      render();
    }
  };
  $('sayForm').onsubmit = function (e) { e.preventDefault(); say(); };
  $('sayText').oninput = function () {
    limit(this, MESSAGE_BYTES, MESSAGE_BYTES);
    renderChat();
  };
  // Keep the chat box above the on-screen keyboard.
  if (window.visualViewport) {
    var view = window.visualViewport, lift = function () {
      $('chat').style.bottom = Math.max(0, window.innerHeight - view.height - view.offsetTop) + 'px';
    };
    view.addEventListener('resize', lift);
    view.addEventListener('scroll', lift);
  }
  document.addEventListener('visibilitychange', function () {
    if (document.hidden) return;
    if (state) showMessages();
    poll();
  });
  poll();
})();
</script>
</body>
</html>
)rawpage";
