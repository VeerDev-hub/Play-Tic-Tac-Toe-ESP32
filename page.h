#pragma once
// The whole game page. Everything is inline because the hotspot has no internet.
// AVATARS, TAUNTS and DARES below must match the counts in game.h.

static const char PAGE[] PROGMEM = R"rawpage(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
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
  .card { position: relative; display: flex; align-items: center; gap: 8px; padding: 8px 10px; border-radius: 12px; border: 3px solid transparent; min-width: 0; }
  .card.s0 { background: var(--p0-tint); }
  .card.s1 { background: var(--p1-tint); }
  .card.s0.active { border-color: var(--p0); }
  .card.s1.active { border-color: var(--p1); }
  .card.away { opacity: .55; }
  .card .face { font-size: 1.8rem; line-height: 1; }
  .card .who { flex: 1; min-width: 0; font-size: .88rem; font-weight: 700; overflow-wrap: anywhere; }
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

  #notice { position: fixed; z-index: 20; left: 12px; right: 12px; bottom: 14px; max-width: 420px; margin: 0 auto; padding: 11px 14px; border-radius: 12px; background: var(--ink); color: #fff; text-align: center; }
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

  @keyframes plop { from { transform: scale(.3); } 70% { transform: scale(1.15); } to { transform: scale(1); } }
  @keyframes wiggle { from { transform: rotate(-14deg); } to { transform: rotate(14deg) scale(1.12); } }
  @keyframes shake { 20%, 60% { transform: translateX(-7px); } 40%, 80% { transform: translateX(7px); } }
  @keyframes fall { to { transform: translateY(115vh) rotate(540deg); } }
  @media (prefers-reduced-motion: reduce) { * { animation: none !important; } .confetti { display: none; } }
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
    <button id="joinBtn" class="primary" disabled>Pick one to join</button>
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

  function name(seat) { return AVATARS[match.players[seat].av][1]; }
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
    var before = match;
    state = next;
    match = next.match;

    if ((match ? match.id : 0) !== shown) {      // a different game, or none
      shown = match ? match.id : 0;
      seenTaunt = [null, null];
      bubbles = [null, null];
      celebrated = null;
    }
    if (before && (!match || match.id !== before.id)) {
      if (before.seat >= 0) {
        if (state.you < 0) notice('You were gone too long, so your game ended.');
        else if (quitting !== before.id) notice(AVATARS[before.players[1 - before.seat].av][1] + ' left the game.');
      } else if (watching && !match) notice('That game is over.');
    }
    if (!match || match.seat >= 0) watching = 0;
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
    });
    if (joining) renderJoin();
    if (match) { checkTaunts(); renderGame(); } else renderLobby();
  }

  function renderJoin() {
    if (picked >= 0 && player(picked)) picked = -1;
    Array.prototype.forEach.call($('picker').children, function (b, i) {
      b.disabled = !!player(i);
      b.setAttribute('aria-pressed', String(i === picked));
    });
    $('joinBtn').disabled = picked < 0;
    setText($('joinBtn'), picked < 0 ? 'Pick one to join' : 'Join as ' + AVATARS[picked][1]);
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
      var text = rival ? 'playing ' + AVATARS[rival.av][1] : 'ready to play';
      if (p === me) text = 'you';
      else if (!p.match) {
        if (me && p.ask === me.av) text = 'wants to play you!';
        else if (p.away) text = 'wandered off';
        else if (me && me.ask === av) text = 'thinking about it...';
        else if (me && state.refusedBy === av) text = 'said no thanks';
      }
      var action = rowAction(p);
      r.row.className = 'row' + (p.away ? ' away' : '');
      setText(r.note, text);
      r.btn.hidden = !action;
      r.btn.className = 'small' + (action === 'ask' ? ' go' : '');
      setText(r.btn, action === 'watch' ? 'Watch' : action === 'cancel' ? 'Cancel'
        : me && state.refusedBy === av ? 'Ask again' : 'Ask to play');
      r.btn.setAttribute('aria-label', r.btn.textContent + ': ' + AVATARS[av][1]);
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
      else text = THINKING[match.round % THINKING.length].replace('{o}', name(opp));
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
      $('dare').appendChild(el('b', '', DARES[match.dare].replace('{w}', name(match.winner))));
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
    var row = el('div', 'row'), who = el('div', 'who', a[1]), note = el('small'), btn = el('button', 'small');
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
    rows.push({ row: row, note: note, btn: btn });

    var card = el('div', 'invite', a[0] + ' ' + a[1] + ' wants to play you!');
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
  $('joinBtn').onclick = function () {
    act('join?avatar=' + picked).then(function (ok) {
      if (ok && state && state.you < 0) notice('Someone beat you to it. Pick another.');
    });
  };
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
  document.addEventListener('visibilitychange', function () { if (!document.hidden) poll(); });
  poll();
})();
</script>
</body>
</html>
)rawpage";
