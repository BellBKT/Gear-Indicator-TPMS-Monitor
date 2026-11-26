// หน้าเว็บของบอร์ด (สร้างจาก ui.src.html) วางไฟล์นี้ไว้โฟลเดอร์เดียวกับ gear_tpms_lvgl.ino
#pragma once
#include <pgmspace.h>
const char WEB_UI[] PROGMEM = R"WEBUI(<!doctype html><html lang="th"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover"><title>Gear TPMS Console</title>
<meta name="claude:viewport-fit-sides" content="cover">
<style>
/* Layout: sticky top tab bar; gear page = H-pattern gate + hall meters (left) and learn panel + table (right); stacks on phones.
   Look: black carbon-fibre weave, amber/cyan/green taken from the in-car display. Self-contained: no external fonts, no CDN. */
:root{
  --bg:#0c0d0e; --panel:rgba(12,13,15,.86); --panel2:rgba(255,255,255,.045);
  --line:rgba(255,255,255,.10); --fg:#ecebe6; --mute:#9094a0;
  --amber:#ffb400; --cyan:#19e3ff; --green:#27f063; --red:#ff4545; --warn:#ffd600;
  --ui:"Segoe UI","Noto Sans Thai","Sarabun","Leelawadee UI","Thonburi",system-ui,-apple-system,sans-serif;
  --num:ui-monospace,"SF Mono",Menlo,Consolas,"Roboto Mono",monospace;
  color-scheme:dark;
}
*{box-sizing:border-box}
html,body{height:auto}
body{
  margin:0; color:var(--fg); font:15px/1.5 var(--ui);
  background-color:#131313;
  background-image:
    linear-gradient(27deg,#151515 5px,transparent 5px),
    linear-gradient(207deg,#151515 5px,transparent 5px),
    linear-gradient(27deg,#222 5px,transparent 5px),
    linear-gradient(207deg,#222 5px,transparent 5px),
    linear-gradient(90deg,#1b1b1b 10px,transparent 10px),
    linear-gradient(#1d1d1d 25%,#1a1a1a 25%,#1a1a1a 50%,transparent 50%,transparent 75%,#242424 75%,#242424);
  background-size:20px 10px,20px 10px,20px 10px,20px 10px,20px 20px,20px 20px;
  background-position:0 5px,10px 0,0 10px,10px 5px,0 0,0 0;
  min-height:100vh;
}
body::before{content:"";position:fixed;inset:0;pointer-events:none;z-index:0;
  background:radial-gradient(120% 80% at 50% -10%,rgba(255,255,255,.07),transparent 55%),radial-gradient(100% 100% at 50% 50%,transparent 40%,rgba(0,0,0,.75));}
[hidden]{display:none!important}
button,input{font:inherit;color:inherit}
.wrap{position:relative;z-index:1;max-width:1040px;margin:0 auto;padding-left:max(16px,env(safe-area-inset-left,0px));padding-right:max(16px,env(safe-area-inset-right,0px));padding-block:0 40px}

/* ---------- top bar ---------- */
.top{position:sticky;top:env(safe-area-inset-top,0px);z-index:20;background:rgba(8,8,9,.92);backdrop-filter:blur(8px);border-bottom:1px solid var(--line);margin-inline:calc(-1 * max(16px,env(safe-area-inset-left,0px)));padding-inline:max(16px,env(safe-area-inset-left,0px))}
.top-in{max-width:1040px;margin:0 auto}
.brand{display:flex;align-items:center;gap:12px;padding:12px 0 6px}
.logo{font:800 15px/1 var(--num);letter-spacing:.16em;color:var(--amber)}
.logo i{font-style:normal;color:var(--fg)}
.sp{flex:1}
.conn{display:flex;align-items:center;gap:8px;font:12px var(--num);color:var(--mute);letter-spacing:.04em}
.dot{width:9px;height:9px;border-radius:50%;background:var(--green);box-shadow:0 0 10px var(--green)}
.conn.bad .dot{background:var(--red);box-shadow:0 0 10px var(--red)}
.conn.sim .dot{background:var(--warn);box-shadow:0 0 10px var(--warn)}
nav.tabs{display:flex;gap:4px;overflow-x:auto;scrollbar-width:none;padding-bottom:0}
nav.tabs::-webkit-scrollbar{display:none}
.tab{flex:0 0 auto;appearance:none;background:none;border:0;border-bottom:3px solid transparent;color:var(--mute);padding:10px 14px 9px;cursor:pointer;display:flex;flex-direction:column;align-items:flex-start;gap:0;text-align:left}
.tab b{font-weight:600;font-size:15px;line-height:1.25}
.tab small{font:10.5px var(--num);letter-spacing:.12em;text-transform:uppercase;opacity:.7}
.tab[aria-selected=true]{color:var(--fg);border-bottom-color:var(--amber)}
.tab:focus-visible,button:focus-visible,input:focus-visible{outline:2px solid var(--cyan);outline-offset:2px}

/* ---------- layout ---------- */
main{padding-top:20px}
.page{display:none}.page.on{display:block}
.grid{display:grid;grid-template-columns:minmax(0,1.05fr) minmax(0,1fr);gap:16px;align-items:start}
.col{display:flex;flex-direction:column;gap:16px;min-width:0}
@media(max-width:820px){.grid{grid-template-columns:minmax(0,1fr)}}
.card{background:var(--panel);border:1px solid var(--line);border-radius:6px;padding:16px;min-width:0;box-shadow:0 10px 30px rgba(0,0,0,.45)}
.card h2{margin:0 0 2px;font:700 12px var(--num);letter-spacing:.16em;text-transform:uppercase;color:var(--amber)}
.card h3{margin:0;font:600 18px/1.3 var(--ui)}
.hd{display:flex;align-items:baseline;gap:10px;flex-wrap:wrap;margin-bottom:12px}
.hd .sp{flex:1}
.muted{color:var(--mute)}
.small{font-size:13px}
.num{font-family:var(--num);font-variant-numeric:tabular-nums}
.pill{display:inline-flex;align-items:center;gap:6px;padding:2px 10px;border-radius:99px;font:600 12px var(--ui);border:1px solid currentColor;white-space:nowrap}
.pill.ok{color:var(--green)}.pill.warn{color:var(--warn)}.pill.bad{color:var(--red)}.pill.off{color:var(--mute)}.pill.info{color:var(--cyan)}

/* ---------- buttons ---------- */
.btn{appearance:none;cursor:pointer;border:1px solid var(--line);background:var(--panel2);border-radius:5px;padding:9px 14px;font-weight:600;min-height:42px}
.btn:hover:not(:disabled){border-color:rgba(255,255,255,.3)}
.btn.pri{background:var(--amber);color:#171000;border-color:var(--amber)}
.btn.pri:hover:not(:disabled){filter:brightness(1.08)}
.btn.dng{color:var(--red);border-color:rgba(255,69,69,.45)}
.btn:disabled{opacity:.4;cursor:not-allowed}
.row{display:flex;gap:10px;flex-wrap:wrap;align-items:center}
.seg{display:inline-flex;border:1px solid var(--line);border-radius:5px;overflow:hidden}
.seg button{appearance:none;border:0;background:none;padding:9px 16px;cursor:pointer;font-weight:600;color:var(--mute);min-height:42px}
.seg button[aria-pressed=true]{background:var(--amber);color:#171000}

/* ---------- gear gate ---------- */
.gate{width:100%;max-width:420px;margin:0 auto;display:block;touch-action:manipulation}
.slot{fill:none;stroke:#050506;stroke-width:36;stroke-linecap:round}
.slot2{fill:none;stroke:#1d1f23;stroke-width:30;stroke-linecap:round}
.slot3{fill:none;stroke:rgba(255,255,255,.05);stroke-width:2;stroke-linecap:round}
.node{cursor:pointer}
.node circle.b{fill:#101114;stroke-width:2.5}
.node text{font:800 26px var(--num);text-anchor:middle;dominant-baseline:central;pointer-events:none}
.node .ring{fill:none;stroke:#fff;stroke-width:2;opacity:0}
.node.sel .ring{opacity:.9}
.node.un circle.b{stroke:var(--red);stroke-dasharray:4 4;opacity:.8}
.node.un text{fill:var(--red);opacity:.8}
.node.cur circle.b{fill:currentColor;fill-opacity:.2}
.node.cur{filter:drop-shadow(0 0 9px currentColor)}
.node.cur text{fill:#fff}
.node .badge{font:700 10px var(--num);fill:var(--mute)}
.knob{transition:transform .22s ease-out;pointer-events:none}
.knob circle{fill:#fff;stroke:#000;stroke-width:2}
.knob .halo{fill:rgba(255,255,255,.18);stroke:none}
@media(prefers-reduced-motion:reduce){.knob{transition:none}}
.legend{display:flex;gap:14px;flex-wrap:wrap;justify-content:center;font-size:12px;color:var(--mute);margin-top:6px}
.legend span{display:inline-flex;align-items:center;gap:6px}
.legend i{width:12px;height:12px;border-radius:50%;border:2px solid;display:inline-block}

/* ---------- hall meters ---------- */
.hall{display:grid;gap:12px}
.hrow{display:grid;grid-template-columns:58px minmax(0,1fr) 64px;gap:10px;align-items:start}
.hrow .lb{padding-top:2px}
.hall .lb{font:12px var(--num);color:var(--mute);line-height:1.2}
.hall .lb b{display:block;color:var(--fg);font-size:14px}
.meter{position:relative;height:22px;background:#07080a;border:1px solid var(--line);border-radius:3px;overflow:hidden}
.meter .fill{position:absolute;left:0;top:0;bottom:0;background:linear-gradient(90deg,rgba(255,180,0,.15),var(--amber));width:0;transition:width .15s linear}
.meter .tick{position:absolute;top:-1px;bottom:-1px;width:3px;margin-left:-1px;background:var(--cyan);box-shadow:0 0 6px var(--cyan);display:none}
.hall .val{font:700 18px var(--num);text-align:right;padding-top:1px}
.scale{display:flex;justify-content:space-between;font:10px var(--num);color:var(--mute);margin-top:3px}
.steady{display:flex;align-items:center;gap:8px;margin-top:12px;font-size:13px}

/* ---------- learn panel / table ---------- */
.sel-title{display:flex;align-items:center;gap:14px}
.big{font:800 54px/1 var(--num)}
.kv{display:grid;grid-template-columns:auto 1fr;gap:4px 14px;font-size:14px;margin:10px 0}
.kv dt{color:var(--mute)}.kv dd{margin:0;font-family:var(--num);font-variant-numeric:tabular-nums}
.steps{margin:0;padding-left:1.2em;color:var(--mute);font-size:13.5px;display:grid;gap:4px}
.steps b{color:var(--fg);font-weight:600}
.result{margin-top:12px;padding:10px 12px;border:1px solid var(--line);border-left:3px solid var(--green);border-radius:4px;background:var(--panel2);font-size:13.5px}
.result.warn{border-left-color:var(--warn)}.result.bad{border-left-color:var(--red)}
.tbl{overflow-x:auto}
table{width:100%;border-collapse:collapse;font:13px var(--num);font-variant-numeric:tabular-nums;min-width:440px}
th{font:600 11px var(--ui);letter-spacing:.06em;color:var(--mute);text-align:right;padding:6px 8px;border-bottom:1px solid var(--line);white-space:nowrap}
th:first-child,td:first-child{text-align:left}
td{padding:7px 8px;text-align:right;border-bottom:1px solid rgba(255,255,255,.05);white-space:nowrap}
tr.cur td{background:rgba(255,180,0,.08)}
tr.pick td:first-child{box-shadow:inset 3px 0 var(--cyan)}
tr{cursor:pointer}
td .g{font-weight:800;font-size:15px}
.dbar{display:inline-block;width:60px;height:6px;background:#07080a;border-radius:3px;vertical-align:middle;margin-left:6px;overflow:hidden}
.dbar i{display:block;height:100%;background:var(--amber)}

/* ---------- tires ---------- */
.tires{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}
.tire{border:1px solid var(--line);border-radius:6px;padding:12px 14px;background:rgba(0,0,0,.35);position:relative;min-width:0}
.tire .pos{font:700 11px var(--num);letter-spacing:.14em;color:var(--mute);text-transform:uppercase}
.tire .p{font:800 34px/1.1 var(--num);margin:4px 0 2px}
.tire .p small{font-size:14px;color:var(--mute);margin-left:4px;font-weight:600}
.tire .mac{font:11px var(--num);color:var(--mute);overflow-wrap:anywhere}
.tire.ok{border-color:rgba(39,240,99,.4)}
.tire.warn{border-color:rgba(255,214,0,.5)}
.tire.bad{border-color:rgba(255,69,69,.6)}
.tire .meta{display:flex;gap:12px;font:12px var(--num);color:var(--mute);margin-top:6px;flex-wrap:wrap}
.wheelrow{display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px}
@media(max-width:560px){.wheelrow{grid-template-columns:repeat(2,minmax(0,1fr))}}
.wchip{border:1px solid var(--line);border-radius:5px;padding:8px 10px;font-size:13px;background:rgba(0,0,0,.3);min-width:0}
.wchip b{display:block;font:700 11px var(--num);letter-spacing:.12em;color:var(--mute)}
.wchip.done{border-color:rgba(39,240,99,.45)}
.wchip.wait{border-color:var(--amber);box-shadow:0 0 0 1px var(--amber) inset}
.field{display:grid;gap:4px}
.field label{font-size:13px;color:var(--mute)}
.inp{width:100%;background:#07080a;border:1px solid var(--line);border-radius:5px;padding:9px 12px;font:700 18px var(--num);min-height:44px}
.fields{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}
@media(max-width:520px){.fields{grid-template-columns:minmax(0,1fr)}}
.note{padding:10px 12px;border-radius:4px;border:1px solid var(--line);background:var(--panel2);font-size:13.5px;color:var(--mute)}
.note b{color:var(--fg)}
.note.warn{border-color:rgba(255,214,0,.4)}
.toast{position:fixed;left:50%;bottom:calc(20px + env(safe-area-inset-bottom,0px));transform:translateX(-50%);z-index:50;background:#000;border:1px solid var(--amber);color:var(--fg);padding:10px 18px;border-radius:99px;font-size:14px;max-width:calc(100vw - 32px)}
.kvt{display:grid;grid-template-columns:minmax(0,max-content) 1fr;gap:6px 18px;font-size:14px}
.kvt dt{color:var(--mute)}.kvt dd{margin:0;font-family:var(--num);overflow-wrap:anywhere}
.prog{height:12px;background:#07080a;border:1px solid var(--line);border-radius:6px;overflow:hidden;margin:8px 0}
.prog i{display:block;height:100%;width:0;background:linear-gradient(90deg,var(--amber),var(--red));transition:width .3s linear}
.rec{display:inline-block;width:10px;height:10px;border-radius:50%;background:var(--red);box-shadow:0 0 10px var(--red);animation:blink 1s infinite;margin-right:8px;vertical-align:middle}
@keyframes blink{50%{opacity:.25}}
@media(prefers-reduced-motion:reduce){.rec{animation:none}}
.capbox{margin-top:12px;padding:12px;border:1px solid var(--red);border-radius:5px;background:rgba(255,69,69,.07)}
.xbtn{appearance:none;border:1px solid var(--line);background:none;color:var(--red);border-radius:4px;padding:2px 10px;cursor:pointer;min-height:30px;font-weight:600}
.risk{display:grid;gap:8px}
.risk .rw{display:grid;grid-template-columns:64px minmax(0,1fr) auto;gap:10px;align-items:center;font-size:13.5px}
.risk .pr{font:800 16px var(--num)}
.simbar{border:1px dashed rgba(255,214,0,.5);border-radius:5px;padding:10px 12px;margin-bottom:16px;font-size:13px;color:var(--mute)}
.simbar .row{margin-top:8px;gap:6px}
.simbar button{min-height:34px;padding:4px 12px;font-family:var(--num)}
</style>
</head><body>
<div class="wrap">
  <header class="top">
    <div class="top-in">
      <div class="brand">
        <div class="logo">GEAR<i>·</i>TPMS</div>
        <div class="sp"></div>
        <div class="conn" id="conn"><span class="dot"></span><span id="connTxt">เชื่อมต่อ</span></div>
      </div>
      <nav class="tabs" role="tablist" aria-label="หน้าหลัก">
        <button class="tab" role="tab" id="t-gear"     data-p="gear"><b>เรียนรู้เกียร์</b><small>Gear learn</small></button>
        <button class="tab" role="tab" id="t-connect"  data-p="connect"><b>เชื่อมต่อ TPMS</b><small>Pair sensors</small></button>
        <button class="tab" role="tab" id="t-settings" data-p="settings"><b>ตั้งค่า TPMS</b><small>Alerts &amp; unit</small></button>
        <button class="tab" role="tab" id="t-system"   data-p="system"><b>ระบบ</b><small>WiFi</small></button>
      </nav>
    </div>
  </header>

  <main>
    <div class="simbar" id="simbar" hidden>
      <b>ตัวอย่างหน้าตา (ข้อมูลจำลอง)</b> หน้านี้ไม่ได้ต่อกับบอร์ด ค่าฮอลล์และล้อเป็นตัวอย่าง เลือกตำแหน่งคันเกียร์จำลองได้ที่นี่
      <div class="row" id="simPos"></div>
    </div>

    <!-- ===================== GEAR ===================== -->
    <section class="page" id="p-gear" aria-labelledby="t-gear">
      <div class="grid">
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>ตำแหน่งคันเกียร์</h2><h3>แผนผังเกียร์ H</h3></div><div class="sp"></div>
              <span class="pill info" id="curPill">ตอนนี้ —</span></div>
            <svg class="gate" id="gate" viewBox="0 0 360 300" role="img" aria-label="แผนผังเกียร์รูปตัว H กดที่ตำแหน่งเพื่อเลือกเกียร์ที่จะเรียนรู้">
              <g>
                <path class="slot"  d="M70 60V240M180 60V240M290 60V240M70 150H290"/>
                <path class="slot2" d="M70 60V240M180 60V240M290 60V240M70 150H290"/>
                <path class="slot3" d="M70 60V240M180 60V240M290 60V240M70 150H290"/>
              </g>
              <g id="nodes"></g>
              <g class="knob" id="knob" style="transform:translate(180px,150px)" hidden>
                <circle class="halo" r="17"/><circle r="8"/>
              </g>
            </svg>
            <div class="legend">
              <span><i style="border-color:var(--amber);background:rgba(255,180,0,.25)"></i>เกียร์ปัจจุบัน</span>
              <span><i style="border-color:var(--mute)"></i>เรียนรู้แล้ว</span>
              <span><i style="border-color:var(--red);border-style:dashed"></i>ยังไม่เรียนรู้</span>
              <span><i style="border-color:#fff;background:#fff;width:9px;height:9px"></i>ตำแหน่งคันตามค่าฮอลล์</span>
            </div>
          </div>

          <div class="card">
            <div class="hd"><div><h2>ค่าฮอลล์สด</h2><h3>Hall sensor</h3></div><div class="sp"></div>
              <span class="muted small">ADC 0-4095 · เฉลี่ย 16 ครั้ง</span></div>
            <div class="hall" id="hall"></div>
            <div class="steady"><span class="pill off" id="steady">—</span><span class="muted" id="steadyTxt"></span></div>
          </div>

          <div class="card">
            <div class="hd"><div><h2>ความเสี่ยงสับสน</h2><h3>คู่เกียร์ที่ช่วงแกว่งใกล้กัน</h3></div></div>
            <div class="risk" id="risk"><span class="muted small">เก็บข้อมูลอย่างน้อย 2 เกียร์ก่อน</span></div>
            <p class="muted small" style="margin:10px 0 0">ใช้ช่วงต่ำสุด–สูงสุดของฮอลล์ทุกรอบ เทียบทีละตัว ถ้ามีฮอลล์ตัวใดที่ช่วงของสองเกียร์ไม่ซ้อนกัน แปลว่าแยกได้ด้วยตัวนั้น ตัวเลขคือระยะห่างระหว่างช่วง (ลบ = ซ้อนทับทุกตัว)</p>
          </div>
        </div>

        <div class="col">
          <div class="card" id="learnCard">
            <div class="hd"><div><h2>เรียนรู้เกียร์</h2><h3>จดจำตำแหน่งที่เลือก</h3></div><div class="sp"></div>
              <span class="pill" id="lockPill">เปิด</span></div>
            <div class="sel-title">
              <div class="big" id="selBig">N</div>
              <div><div id="selState" class="small"></div><div class="muted small" id="selHint">กดที่ตำแหน่งบนแผนผังหรือในตาราง</div></div>
            </div>
            <dl class="kv" id="selKv"></dl>
            <div class="field" style="margin-bottom:12px"><label>เวลาเก็บต่อ 1 รอบ</label>
              <div class="seg" id="durSeg"><button data-v="5">5 วิ</button><button data-v="10">10 วิ</button><button data-v="20">20 วิ</button></div></div>
            <div class="row">
              <button class="btn pri" id="btnLearn">เก็บรอบแรก</button>
              <button class="btn" id="btnLock">ล็อกการเรียนรู้</button>
            </div>
            <div class="capbox" id="capBox" hidden>
              <div><span class="rec"></span><b id="capTitle">กำลังเก็บ</b></div>
              <div class="prog"><i id="capBar"></i></div>
              <div class="small muted num" id="capLive"></div>
              <div class="row" style="margin-top:10px"><button class="btn dng" id="btnCancel">ยกเลิกรอบนี้</button></div>
            </div>
            <div class="result" id="lrnRes" hidden></div>
            <ol class="steps" style="margin-top:14px">
              <li><b>ต่อบอร์ดกับไฟในรถ</b> แล้วเชื่อม WiFi <span class="num">GearTPMS</span></li>
              <li><b>เลือกเกียร์</b> กด “เก็บ” แล้วขับตามปกติจนครบเวลา (ให้ผู้โดยสารกด หรือกดก่อนออกตัว)</li>
              <li><b>เก็บซ้ำหลายรอบ</b> ทุกเกียร์ รอบละหลายครั้งตามจังหวะขับจริง ทั้ง N→1, N→R, 5→2 ฯลฯ แนะนำ 5-6 รอบ (เก็บได้สูงสุด 10)</li>
              <li><b>ดูตารางและความเสี่ยง</b> แล้วกดล็อกการเรียนรู้</li>
            </ol>
          </div>

          <div class="card" id="runsCard">
            <div class="hd"><div><h2>ข้อมูลที่เก็บไว้</h2><h3 id="runsTitle">เกียร์ N</h3></div><div class="sp"></div><span class="pill off" id="runsPill">0/10 รอบ</span></div>
            <div id="runsSum" class="small" style="margin-bottom:10px"></div>
            <div class="tbl"><table id="runsTbl"><thead><tr><th>รอบ</th><th>H0 เฉลี่ย</th><th>H1 เฉลี่ย</th><th>H2 เฉลี่ย</th><th>แกว่งในรอบ</th><th></th></tr></thead><tbody></tbody></table></div>
            <p class="muted small" style="margin:10px 0 0" id="runsNote"></p>
            <div class="row" style="margin-top:10px"><button class="btn dng" id="btnGearClear">ลบข้อมูลของเกียร์นี้</button></div>
          </div>

          <div class="card">
            <div class="hd"><div><h2>ตารางค่าที่เรียนรู้</h2><h3>ค่าเก็บไว้เทียบกับค่าสด</h3></div><div class="sp"></div>
              <span class="pill off" id="cnt">0/7</span></div>
            <div class="tbl"><table id="tbl"><thead><tr><th>เกียร์</th><th>รอบ</th><th>H0 บน</th><th>H1 กลาง</th><th>H2 ล่าง</th><th>ห่างจากค่าสด</th><th>ห่างจากเกียร์อื่น</th></tr></thead><tbody></tbody></table></div>
            <p class="muted small" style="margin:10px 0 0">ช่อง “ห่างจากเกียร์อื่น” คือระยะถึงเกียร์ที่ใกล้ที่สุด ยิ่งมากยิ่งแยกง่าย คู่ที่ต้องดู: 3 กับ 5, 4 กับ R</p>
            <div class="row" style="margin-top:12px">
              <button class="btn dng" id="btnClear">ลบค่าที่เรียนรู้ทั้งหมด</button>
              <span class="muted small" id="clearHint"></span>
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- ===================== TPMS CONNECT ===================== -->
    <section class="page" id="p-connect" aria-labelledby="t-connect">
      <div class="grid">
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>สถานะล้อ</h2><h3>ล้อทั้ง 4</h3></div><div class="sp"></div><span class="pill off" id="modePill">—</span></div>
            <div class="tires" id="tires"></div>
          </div>
        </div>
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>โหมดการทำงาน</h2><h3>จำลอง หรือ เซนเซอร์จริง</h3></div></div>
            <div class="row"><div class="seg" id="modeSeg"><button data-v="1">DEMO จำลอง</button><button data-v="0">REAL เซนเซอร์จริง</button></div></div>
            <p class="muted small" style="margin:10px 0 0">เปลี่ยนโหมดแล้วบอร์ดจะรีสตาร์ท หน้านี้จะเชื่อมต่อใหม่เอง ค่าเกียร์ที่เรียนรู้ไม่หาย</p>
          </div>
          <div class="card">
            <div class="hd"><div><h2>จับคู่เซนเซอร์</h2><h3>ล้อหน้าซ้าย → หน้าขวา → หลังซ้าย → หลังขวา</h3></div></div>
            <div class="wheelrow" id="pairRow"></div>
            <p class="note" style="margin:12px 0" id="pairNote"></p>
            <div class="row">
              <button class="btn pri" id="pairStart">เริ่มจับคู่</button>
              <button class="btn" id="pairSkip">ข้ามล้อนี้</button>
              <button class="btn dng" id="pairCancel">ยกเลิก</button>
            </div>
          </div>
        </div>
      </div>
    </section>

    <!-- ===================== TPMS SETTINGS ===================== -->
    <section class="page" id="p-settings" aria-labelledby="t-settings">
      <div class="grid">
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>หน่วยความดัน</h2><h3>แสดงบนจอและหน้านี้</h3></div></div>
            <div class="seg" id="unitSeg"><button data-v="0">PSI</button><button data-v="1">BAR</button></div>
          </div>
          <div class="card">
            <div class="hd"><div><h2>เกณฑ์แจ้งเตือน</h2><h3>ความดันลมยาง</h3></div><div class="sp"></div><span class="muted small" id="unitHint"></span></div>
            <div class="fields">
              <div class="field"><label for="fLo">ต่ำ (แดง) ต่ำกว่านี้</label><input class="inp" id="fLo" type="number" inputmode="decimal" step="any"></div>
              <div class="field"><label for="fWa">เตือน (เหลือง) ต่ำกว่านี้</label><input class="inp" id="fWa" type="number" inputmode="decimal" step="any"></div>
              <div class="field"><label for="fHi">สูง (แดง) สูงกว่านี้</label><input class="inp" id="fHi" type="number" inputmode="decimal" step="any"></div>
            </div>
            <p class="note warn" id="setErr" style="margin:12px 0 0" hidden></p>
            <div class="row" style="margin-top:14px">
              <button class="btn pri" id="setSave">บันทึกเกณฑ์</button>
              <button class="btn" id="setDef">ใช้ค่าเริ่มต้น</button>
            </div>
          </div>
        </div>
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>ตัวอย่างผลต่อสถานะ</h2><h3>ค่าตอนนี้แปลเป็น</h3></div></div>
            <div id="bands" class="small" style="display:grid;gap:8px"></div>
          </div>
          <div class="card">
            <div class="hd"><div><h2>เกณฑ์ที่ตายตัว</h2><h3>ปรับในโค้ดเท่านั้น</h3></div></div>
            <dl class="kvt"><dt>อุณหภูมิสูง</dt><dd>&gt; 70 °C</dd><dt>แบตเซนเซอร์ต่ำ</dt><dd>&lt; 20 %</dd><dt>ไม่มีสัญญาณ</dt><dd>เกิน 5 นาที</dd></dl>
          </div>
        </div>
      </div>
    </section>

    <!-- ===================== SYSTEM ===================== -->
    <section class="page" id="p-system" aria-labelledby="t-system">
      <div class="grid">
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>WiFi ของบอร์ด</h2><h3>เชื่อมต่อจากมือถือ</h3></div></div>
            <dl class="kvt" id="sysWifi"></dl>
            <div class="row" style="margin-top:14px">
              <button class="btn dng" id="wifiOff">ปิด WiFi ตอนนี้</button>
            </div>
            <p class="muted small" style="margin:10px 0 0">WiFi ปิดเองเมื่อไม่มีการใช้งานครบเวลา เปิดใหม่ได้จากเมนูบนจอ (MENU → WiFi)</p>
          </div>
        </div>
        <div class="col">
          <div class="card">
            <div class="hd"><div><h2>สถานะบอร์ด</h2><h3>ESP32</h3></div></div>
            <dl class="kvt" id="sysInfo"></dl>
          </div>
          <div class="card">
            <div class="hd"><div><h2>ข้อควรรู้</h2><h3>ระหว่างเปิด WiFi</h3></div></div>
            <ul class="steps" style="list-style:disc">
              <li>ความสว่างไฟ LED ถูกค้างไว้ เพราะขา LDR อ่านไม่ได้ตอนเปิด WiFi</li>
              <li>เรียนรู้เกียร์ตอนต่อไฟรถจริง ค่าจะตรงกับตอนใช้งาน</li>
              <li>เรียนรู้เสร็จแล้วกด “ล็อก” แล้วปิด WiFi</li>
            </ul>
          </div>
        </div>
      </div>
    </section>
  </main>
</div>
<div class="toast" id="toast" hidden></div>

<script>
(function(){
'use strict';
const SIM = location.protocol !== 'http:';          // เปิดจากบอร์ดคือ http:// ถ้าไม่ใช่ = โหมดตัวอย่าง
const GN = ['N','1','2','3','4','5','R'];            // ลำดับตามตารางในบอร์ด: 0=N 1-5 6=R
const GCOL = ['var(--green)','var(--amber)','var(--amber)','var(--amber)','var(--amber)','var(--amber)','var(--cyan)'];
const POS = [[180,150],[70,60],[70,240],[180,60],[180,240],[290,60],[290,240]];
const WHEEL = [['FL','หน้าซ้าย'],['FR','หน้าขวา'],['RL','หลังซ้าย'],['RR','หลังขวา']];
const ST = [['ไม่มีสัญญาณ','off'],['ปกติ','ok'],['ควรตรวจ','warn'],['ลมอ่อน','bad'],['ลมสูง','bad'],['ร้อนจัด','bad'],['แบตต่ำ','warn'],['เตือนจากเซนเซอร์','bad']];
const DEF = {lo:200, wa:220, hi:320};
const $ = id => document.getElementById(id);
const el = (t,c,h) => { const e=document.createElement(t); if(c) e.className=c; if(h!=null) e.innerHTML=h; return e; };

let S = null, selIdx = 0, hist = [], failCount = 0, clearArmed = false, gClrArmed = false;
let G = null, R = {}, durSel = 10, lastSeq = null, lastRc = '';
let settingsDirty = false;

/* ---------------- ตัวจำลอง (ใช้เฉพาะตอนดูตัวอย่างนอกบอร์ด) ---------------- */
const sim = {
  base: [[1240,1271,1250],[1480,1180,990],[1500,1190,1520],[1330,1480,1260],[1360,1500,1520],null,null],
  v:[null,null,null,null,null,null,null], runs:[[],[],[],[],[],[],[]],
  lever: 0, h:[1240,1271,1250], lrn:true, bar:false, demo:true, pair:-1, t0:Date.now(),
  tp:{lo:200,wa:220,hi:320}, cap:null, seq:0, lastOk:true, lastIdx:0,
  tires:[{k:235,c:31,b:90},{k:210,c:32,b:65},{k:232,c:33,b:40},{k:190,c:34,b:15}],
  macs:['a1:b2:c3:d4:e5:01','a1:b2:c3:d4:e5:02','',''],
  init(){
    const cnt=[6,5,4,5,5,0,0];
    this.base.forEach((b,i)=>{ if(!b) return;
      for(let r=0;r<cnt[i];r++){ const m=b.map(x=>Math.round(x+(Math.random()*16-8)));
        this.runs[i].push(m.concat(m.map(x=>x-Math.round(6+Math.random()*10)),m.map(x=>x+Math.round(6+Math.random()*10)))); }
      this.recalc(i); });
  },
  med(a){ const b=a.slice().sort((x,y)=>x-y), n=b.length; return n%2?b[(n-1)/2]:Math.round((b[n/2-1]+b[n/2])/2); },
  recalc(i){ const r=this.runs[i]; this.v[i]=r.length?[0,1,2].map(k=>this.med(r.map(x=>x[k]))):null; },
  leverVec(){ const v=this.v[this.lever]; if(v) return v;
    const base=this.v[0]; return this.lever===5?[base[0]+130,base[1]+10,base[2]-80]:[base[0]+140,base[1]+20,base[2]+110]; },
  tick(){ const t=this.leverVec(); this.h=this.h.map((x,i)=>Math.round(x+(t[i]-x)*0.35+(Math.random()*14-7))); },
  capStep(){
    const c=this.cap; if(!c) return;
    c.n++; this.h.forEach((x,k)=>{ c.sum[k]+=x; c.lo[k]=Math.min(c.lo[k],x); c.hi[k]=Math.max(c.hi[k],x); });
    if(Date.now()-c.t0>=c.d){ const m=c.sum.map(x=>Math.round(x/c.n));
      if(this.runs[c.i].length>=10) this.runs[c.i].shift();
      this.runs[c.i].push(m.concat(c.lo,c.hi)); this.recalc(c.i); this.lastIdx=c.i; this.lastOk=true; this.seq++; this.cap=null; }
  },
  state(){
    this.tick(); this.capStep();
    let mask=0; this.v.forEach((x,i)=>{ if(x) mask|=1<<i; });
    const c=this.cap;
    return { h:this.h, g:this.curGear(), m:mask, v:this.v.map(x=>x||[0,0,0]), lrn:this.lrn, rc:this.runs.map(r=>r.length),
      ln:{on:c?1:0,i:c?c.i:0,t:c?Date.now()-c.t0:0,d:c?c.d:10000,n:c?c.n:0,seq:this.seq,ok:this.lastOk?1:0,li:this.lastIdx,lo:c?c.lo:[0,0,0],hi:c?c.hi:[0,0,0]},
      tp:{demo:this.demo,bar:this.bar,lo:this.tp.lo,wa:this.tp.wa,hi:this.tp.hi,pair:this.pair,
          tires:this.tires.map((x,i)=>({k:+(x.k+Math.random()*2-1).toFixed(1),c:x.c,b:x.b,s:this.status(x),m:this.macs[i],a:3}))},
      sys:{ssid:'GearTPMS',pw:'12345678',ip:'192.168.4.1',cl:1,off:842,heap:118432,up:Math.round((Date.now()-this.t0)/1000),ver:'preview'} };
  },
  get(path){
    if(path==='/api/gears') return {g:this.runs.map((r,i)=>{ const c=this.v[i]||[0,0,0];
      return {n:r.length,c:c,mn:[0,1,2].map(k=>r.length?Math.min(...r.map(x=>x[3+k])):c[k]),mx:[0,1,2].map(k=>r.length?Math.max(...r.map(x=>x[6+k])):c[k])}; })};
    const m=path.match(/runs\?i=(\d)/); if(m) return {i:+m[1],r:this.runs[+m[1]]};
    return {};
  },
  curGear(){ let best=-1,bd=1e9; this.v.forEach((x,i)=>{ if(!x) return; const d=Math.hypot(x[0]-this.h[0],x[1]-this.h[1],x[2]-this.h[2]); if(d<bd){bd=d;best=i;} }); return bd<250?(best===6?-1:best):null; },
  status(x){ if(x.k>this.tp.hi) return 4; if(x.k<this.tp.lo) return 3; if(x.c>70) return 5; if(x.b<20) return 6; if(x.k<this.tp.wa) return 2; return 1; },
  async post(path,p){
    if(path==='/api/learn'){
      if(this.cap) return {ok:false,err:'busy'};
      this.cap={i:+p.i,d:(+p.s||10)*1000,t0:Date.now(),n:0,sum:[0,0,0],lo:[9999,9999,9999],hi:[-1,-1,-1]}; return {ok:true};
    }
    if(path==='/api/learncancel'){ this.cap=null; return {ok:true}; }
    if(path==='/api/rundel'){ this.runs[+p.i].splice(+p.r,1); this.recalc(+p.i); return {ok:true}; }
    if(path==='/api/gearclear'){ this.runs[+p.i]=[]; this.recalc(+p.i); return {ok:true}; }
    if(path==='/api/clear'){ this.runs=[[],[],[],[],[],[],[]]; this.v=[null,null,null,null,null,null,null]; return {ok:true}; }
    if(path==='/api/lock'){ this.lrn = p.on==='1'; return {ok:true}; }
    if(path==='/api/unit'){ this.bar = p.bar==='1'; return {ok:true}; }
    if(path==='/api/mode'){ this.demo = p.demo==='1'; return {ok:true,restart:true}; }
    if(path==='/api/tpms'){ this.tp={lo:+p.lo,wa:+p.wa,hi:+p.hi}; return {ok:true}; }
    if(path==='/api/pair'){
      if(p.a==='start'){ if(this.demo) return {ok:false,err:'demo'}; this.pair=0; }
      else if(p.a==='skip'){ this.pair++; if(this.pair>3) this.pair=-1; }
      else this.pair=-1;
      if(this.pair>=0 && !this.simTimer){ this.simTimer=setInterval(()=>{ if(this.pair<0){clearInterval(this.simTimer);this.simTimer=0;return;} this.macs[this.pair]='a1:b2:c3:d4:e5:0'+(this.pair+1); this.pair++; if(this.pair>3) this.pair=-1; },3500); }
      return {ok:true};
    }
    if(path==='/api/wifi'){ return {ok:true}; }
    return {ok:false};
  }
};
sim.init();
function nearestOther(v,idx){
  let best=-1,bd=1e9;
  v.forEach((x,i)=>{ if(!x||i===idx||!v[idx]) return; const d=Math.hypot(x[0]-v[idx][0],x[1]-v[idx][1],x[2]-v[idx][2]); if(d<bd){bd=d;best=i;} });
  return best<0?null:{i:best,d:Math.round(bd)};
}

/* ---------------- เรียกบอร์ด ---------------- */
async function getState(){
  if(SIM) return sim.state();
  const r = await fetch('/api/state',{cache:'no-store'});
  if(!r.ok) throw new Error(r.status);
  return r.json();
}
async function getJson(path){
  if(SIM) return sim.get(path);
  const r = await fetch(path,{cache:'no-store'});
  if(!r.ok) throw new Error(r.status);
  return r.json();
}
async function post(path,params){
  if(SIM) return sim.post(path,params||{});
  const r = await fetch(path,{method:'POST',body:new URLSearchParams(params||{})});
  return r.json();
}
function toast(msg){ const t=$('toast'); t.textContent=msg; t.hidden=false; clearTimeout(toast.k); toast.k=setTimeout(()=>{t.hidden=true;},2600); }

/* ---------------- ช่วยคำนวณ ---------------- */
const learned = i => !!(S && (S.m>>i)&1);
const dist = (a,b) => Math.hypot(a[0]-b[0],a[1]-b[1],a[2]-b[2]);
const toUnit = k => S.tp.bar ? k/100 : k*0.145038;
const fromUnit = v => S.tp.bar ? v*100 : v/0.145038;
const fmtU = k => S.tp.bar ? (k/100).toFixed(2) : (k*0.145038).toFixed(1);
const uName = () => S.tp.bar ? 'bar' : 'psi';
const gIdx = g => g===-1 ? 6 : g;

/* ---------------- สร้างหน้าจอส่วนที่คงที่ ---------------- */
function buildGate(){
  const g=$('nodes');
  GN.forEach((n,i)=>{
    const [x,y]=POS[i];
    const nd=document.createElementNS('http://www.w3.org/2000/svg','g');
    nd.setAttribute('class','node'); nd.setAttribute('tabindex','0'); nd.setAttribute('role','button');
    nd.setAttribute('aria-label','เกียร์ '+n); nd.style.color=GCOL[i]; nd.dataset.i=i;
    nd.innerHTML='<circle class="ring" cx="'+x+'" cy="'+y+'" r="33"/><circle class="b" cx="'+x+'" cy="'+y+'" r="27" style="stroke:'+GCOL[i]+'"/><text x="'+x+'" y="'+y+'" fill="'+GCOL[i]+'">'+n+'</text>';
    nd.addEventListener('click',()=>selectGear(i));
    nd.addEventListener('keydown',e=>{ if(e.key==='Enter'||e.key===' '){e.preventDefault();selectGear(i);} });
    g.appendChild(nd);
  });
  const h=$('hall');
  ['H0|บน','H1|กลาง','H2|ล่าง'].forEach((s,i)=>{
    const [a,b]=s.split('|');
    const r=el('div','hrow','<div class="lb"><b>'+a+'</b>'+b+'</div><div><div class="meter"><div class="fill" id="hf'+i+'"></div><div class="tick" id="ht'+i+'"></div></div><div class="scale"><span id="hl'+i+'"></span><span id="hh'+i+'"></span></div></div><div class="val num" id="hv'+i+'">—</div>');
    h.appendChild(r);
  });
  const tb=$('tbl').querySelector('tbody');
  GN.forEach((n,i)=>{
    const tr=el('tr',null,'<td><span class="g" style="color:'+GCOL[i]+'">'+n+'</span></td><td></td><td></td><td></td><td></td><td></td><td></td>');
    tr.id='tr'+i; tr.addEventListener('click',()=>selectGear(i)); tb.appendChild(tr);
  });
  if(SIM){
    $('simbar').hidden=false;
    const row=$('simPos');
    GN.forEach((n,i)=>{ const b=el('button','btn',n); b.addEventListener('click',()=>{ sim.lever=i; }); row.appendChild(b); });
  }
}

/* ---------------- หน้าเกียร์ ---------------- */
function selectGear(i){ selIdx=i; $('lrnRes').hidden=true; gClrArmed=false; if(S) renderGear(); loadRuns(i); }

function leverPos(){
  let sx=0,sy=0,sw=0;
  for(let i=0;i<7;i++){ if(!learned(i)) continue;
    const d=dist(S.h,S.v[i]); const w=1/(d*d+400); sx+=POS[i][0]*w; sy+=POS[i][1]*w; sw+=w; }
  return sw? [sx/sw,sy/sw] : null;
}

function renderGear(){
  const cur = (S.g==null) ? -2 : gIdx(S.g);
  document.querySelectorAll('.node').forEach(nd=>{
    const i=+nd.dataset.i;
    nd.classList.toggle('un',!learned(i));
    nd.classList.toggle('cur',i===cur);
    nd.classList.toggle('sel',i===selIdx);
  });
  const lp=leverPos(), kn=$('knob');
  kn.hidden=!lp;
  if(lp) kn.style.transform='translate('+lp[0].toFixed(1)+'px,'+lp[1].toFixed(1)+'px)';
  const cp=$('curPill');
  cp.textContent = cur===-2 ? 'ตอนนี้ ไม่ทราบตำแหน่ง' : 'ตอนนี้ เกียร์ '+GN[cur];
  cp.className='pill '+(cur===-2?'off':'info');

  // ฮอลล์: ช่วงแสดงปรับตามค่าที่เรียนรู้ไว้ + ค่าสด
  const vals=[]; for(let i=0;i<7;i++) if(learned(i)) vals.push(...S.v[i]);
  vals.push(...S.h);
  for(let k=0;k<3;k++){
    const col=[S.h[k]]; for(let i=0;i<7;i++) if(learned(i)) col.push(S.v[i][k]);
    let lo=Math.min(...col)-80, hi=Math.max(...col)+80;
    if(hi-lo<300){ const c=(hi+lo)/2; lo=c-150; hi=c+150; }
    lo=Math.max(0,Math.floor(lo/10)*10); hi=Math.min(4095,Math.ceil(hi/10)*10);
    const pct=v=>Math.max(0,Math.min(100,(v-lo)/(hi-lo)*100));
    $('hf'+k).style.width=pct(S.h[k])+'%';
    $('hv'+k).textContent=S.h[k];
    $('hl'+k).textContent=lo; $('hh'+k).textContent=hi;
    const tk=$('ht'+k);
    if(learned(selIdx)){ tk.style.display='block'; tk.style.left=pct(S.v[selIdx][k])+'%'; } else tk.style.display='none';
  }

  // ความนิ่ง
  hist.push(S.h.slice()); if(hist.length>12) hist.shift();
  let sp=0; if(hist.length>=6){ for(let k=0;k<3;k++){ const a=hist.map(x=>x[k]); sp=Math.max(sp,Math.max(...a)-Math.min(...a)); } }
  const st=$('steady');
  if(hist.length<6){ st.textContent='กำลังวัด'; st.className='pill off'; $('steadyTxt').textContent=''; }
  else if(sp<=12){ st.textContent='นิ่ง'; st.className='pill ok'; $('steadyTxt').textContent='แกว่ง ±'+Math.ceil(sp/2)+' พร้อมเรียนรู้'; }
  else { st.textContent='ยังแกว่ง'; st.className='pill warn'; $('steadyTxt').textContent='แกว่ง '+sp+' จับคันเกียร์ให้นิ่งก่อนกดเรียนรู้'; }

  // แผงเรียนรู้
  $('selBig').textContent=GN[selIdx]; $('selBig').style.color=GCOL[selIdx];
  const L=learned(selIdx);
  $('selState').innerHTML = L ? '<span class="pill ok">เรียนรู้แล้ว</span>' : '<span class="pill bad">ยังไม่เรียนรู้</span>';
  $('selHint').textContent = selIdx===5||selIdx===6 ? 'ตำแหน่งปลายคัน สัญญาณอ่อนกว่าตำแหน่งอื่น' : 'เข้าเกียร์นี้ จับให้นิ่ง แล้วกดเรียนรู้';
  const kv=$('selKv'); kv.innerHTML='';
  const row=(a,b)=>{ kv.appendChild(el('dt',null,a)); kv.appendChild(el('dd',null,b)); };
  row('ค่าที่เก็บไว้', L ? S.v[selIdx].join(' · ') : '—');
  row('ค่าสดตอนนี้', S.h.join(' · '));
  row('ต่างจากที่เก็บ', L ? S.h.map((x,k)=>{ const d=x-S.v[selIdx][k]; return (d>0?'+':'')+d; }).join(' · ') : '—');
  const ln=S.ln, capOn=ln.on===1, nr=S.rc[selIdx];
  $('btnLearn').disabled = !S.lrn || capOn;
  $('btnLearn').textContent = (nr>0?'เก็บเพิ่มอีก 1 รอบ':'เก็บรอบแรก')+' · '+durSel+' วิ';
  document.querySelectorAll('#durSeg button').forEach(b=>b.setAttribute('aria-pressed',String(+b.dataset.v===durSel)));
  $('capBox').hidden=!capOn;
  if(capOn){
    const left=Math.max(0,Math.ceil((ln.d-ln.t)/1000));
    $('capTitle').textContent='กำลังเก็บเกียร์ '+GN[ln.i]+' · เหลือ '+left+' วินาที';
    $('capBar').style.width=Math.min(100,ln.t/ln.d*100)+'%';
    $('capLive').textContent='ตัวอย่าง '+ln.n+' · ช่วงที่ค่าแกว่ง H0 '+ln.lo[0]+'–'+ln.hi[0]+' · H1 '+ln.lo[1]+'–'+ln.hi[1]+' · H2 '+ln.lo[2]+'–'+ln.hi[2];
  }
  const lp2=$('lockPill'); lp2.textContent=S.lrn?'เปิดเรียนรู้':'ล็อกอยู่'; lp2.className='pill '+(S.lrn?'warn':'ok');
  $('btnLock').textContent=S.lrn?'ล็อกการเรียนรู้':'เปิดการเรียนรู้';
  $('btnClear').disabled=!S.lrn || !S.m || capOn;
  $('btnGearClear').disabled=!S.lrn || capOn || (!nr && !L);
  $('clearHint').textContent = !S.lrn ? 'ต้องเปิดการเรียนรู้ก่อนจึงลบได้' : '';
  if(!clearArmed) $('btnClear').textContent='ลบค่าที่เรียนรู้ทั้งหมด';

  // ตาราง
  let n=0;
  for(let i=0;i<7;i++){
    const tr=$('tr'+i), td=tr.children;
    tr.classList.toggle('cur',i===cur); tr.classList.toggle('pick',i===selIdx);
    const rc=S.rc[i];
    td[1].innerHTML=rc? '<span class="pill '+(rc<5?'warn':'ok')+'">'+rc+'</span>' : '—';
    if(!learned(i)){
      td[2].textContent=td[3].textContent=td[4].textContent='—';
      td[5].textContent='—'; td[6].innerHTML='<span class="pill bad">ยังไม่เรียนรู้</span>';
      continue;
    }
    n++;
    td[2].textContent=S.v[i][0]; td[3].textContent=S.v[i][1]; td[4].textContent=S.v[i][2];
    const d=Math.round(dist(S.h,S.v[i]));
    td[5].innerHTML=d+'<span class="dbar"><i style="width:'+Math.min(100,d/3)+'%"></i></span>';
    const o=nearestOther(S.v.map((x,k)=>learned(k)?x:null),i);
    if(!o) td[6].textContent='—';
    else { const c=o.d<30?'bad':o.d<60?'warn':'ok'; td[6].innerHTML='<span class="pill '+c+'">'+o.d+' → '+GN[o.i]+'</span>'; }
  }
  $('cnt').textContent=n+'/7'; $('cnt').className='pill '+(n===7?'ok':'warn');
}

async function doLearn(){
  if(!S.lrn||S.ln.on) return;
  try{
    const r=await post('/api/learn',{i:selIdx,s:durSel});
    if(!r.ok) toast(r.err==='busy'?'กำลังเก็บอยู่แล้ว':'การเรียนรู้ถูกล็อก'); else $('lrnRes').hidden=true;
  }catch(e){ toast('ติดต่อบอร์ดไม่ได้'); }
  refresh();
}

async function loadGears(){ try{ G=(await getJson('/api/gears')).g; }catch(e){} renderRisk(); }
async function loadRuns(i){ try{ R[i]=(await getJson('/api/runs?i='+i)).r; }catch(e){} renderRuns(); }

function renderRuns(){
  if(!S) return;
  const i=selIdx, rr=R[i]||[], n=S.rc[i];
  $('runsTitle').textContent='เกียร์ '+GN[i]; $('runsTitle').style.color=GCOL[i];
  const pl=$('runsPill'); pl.textContent=n+'/10 รอบ'; pl.className='pill '+(n===0?'off':n<5?'warn':'ok');
  const sum=$('runsSum');
  if(n>0 && rr.length===n){
    const sp=[0,1,2].map(k=>{ const a=rr.map(x=>x[k]); return Math.max(...a)-Math.min(...a); });
    const sw=[0,1,2].map(k=>Math.round(rr.reduce((t,x)=>t+(x[6+k]-x[3+k]),0)/rr.length));
    sum.innerHTML='ค่ากลาง (มัธยฐาน) <b class="num">'+S.v[i].join(' · ')+'</b><br><span class="muted">ค่าเฉลี่ยของแต่ละรอบต่างกันได้ '+sp.join(' / ')+' · แกว่งในรอบเฉลี่ย '+sw.join(' / ')+'</span>';
  } else if(n===0 && learned(i)) sum.innerHTML='ใช้ค่าเดิมจากการเรียนรู้แบบครั้งเดียว <b class="num">'+S.v[i].join(' · ')+'</b> เก็บรอบใหม่เพื่อแทนที่ด้วยค่ากลางจากหลายรอบ';
  else sum.innerHTML='<span class="muted">ยังไม่มีข้อมูลเกียร์นี้</span>';
  const tb=$('runsTbl').querySelector('tbody'); tb.innerHTML='';
  if(rr.length===n) rr.forEach((x,k)=>{
    const tr=el('tr',null,'<td>'+(k+1)+'</td><td>'+x[0]+'</td><td>'+x[1]+'</td><td>'+x[2]+'</td><td>'+(x[6]-x[3])+' / '+(x[7]-x[4])+' / '+(x[8]-x[5])+'</td><td><button class="xbtn" data-r="'+k+'"'+(S.lrn?'':' disabled')+'>ลบ</button></td>');
    tr.style.cursor='default'; tb.appendChild(tr);
  });
  $('runsNote').textContent = n===0?'' : n<5?'เก็บอย่างน้อย 5 รอบ จังหวะขับคนละแบบ จะได้ค่ากลางที่เชื่อถือได้' : n>=10?'เต็ม 10 รอบ เก็บเพิ่มจะแทนรอบเก่าสุด':'พอใช้ได้แล้ว เก็บเพิ่มได้ถึง 10 รอบ';
  if(!gClrArmed) $('btnGearClear').textContent='ลบข้อมูลของเกียร์นี้';
}

function renderRisk(){
  const box=$('risk');
  if(!S||!G){ return; }
  const ids=[]; for(let i=0;i<7;i++) if(learned(i)) ids.push(i);
  if(ids.length<2){ box.innerHTML='<span class="muted small">เก็บข้อมูลอย่างน้อย 2 เกียร์ก่อน</span>'; return; }
  const pairs=[];
  for(let x=0;x<ids.length;x++) for(let y=x+1;y<ids.length;y++){
    const a=G[ids[x]], b=G[ids[y]]; let best=-1e9, bk=0;
    for(let k=0;k<3;k++){ const gap=Math.max(b.mn[k]-a.mx[k], a.mn[k]-b.mx[k]); if(gap>best){best=gap;bk=k;} }
    pairs.push({a:ids[x],b:ids[y],gap:best,k:bk,few:(a.n<3||b.n<3)});
  }
  pairs.sort((p,q)=>p.gap-q.gap);
  box.innerHTML='';
  pairs.slice(0,8).forEach(p=>{
    const c=p.gap>=40?'ok':p.gap>=0?'warn':'bad', t=p.gap>=40?'แยกได้':p.gap>=0?'แคบ':'ซ้อนกัน';
    const txt=p.gap>=0?'แยกด้วย H'+p.k+' ห่าง '+p.gap:'ซ้อนกันทุกฮอลล์ (ตื้นสุด '+(-p.gap)+' ที่ H'+p.k+')';
    box.appendChild(el('div','rw','<div class="pr"><span style="color:'+GCOL[p.a]+'">'+GN[p.a]+'</span> ↔ <span style="color:'+GCOL[p.b]+'">'+GN[p.b]+'</span></div><div>'+txt+(p.few?' <span class="muted">(ข้อมูลน้อย)</span>':'')+'</div><span class="pill '+c+'">'+t+'</span>'));
  });
}

function capResult(){
  const i=S.ln.li, box=$('lrnRes'); box.hidden=false;
  if(S.ln.ok){ box.className='result'; box.innerHTML='เก็บรอบใหม่ของเกียร์ <b>'+GN[i]+'</b> แล้ว รวมตอนนี้ '+S.rc[i]+' รอบ'; if(i!==selIdx) selectGear(i); }
  else { box.className='result bad'; box.textContent='เก็บไม่สำเร็จ ข้อมูลไม่พอ ลองใหม่'; }
}

/* ---------------- หน้า TPMS ---------------- */
function renderTpms(){
  const T=S.tp;
  const mp=$('modePill'); mp.textContent=T.demo?'DEMO ข้อมูลจำลอง':'REAL เซนเซอร์จริง'; mp.className='pill '+(T.demo?'warn':'ok');
  document.querySelectorAll('#modeSeg button').forEach(b=>b.setAttribute('aria-pressed',String((b.dataset.v==='1')===T.demo)));
  const box=$('tires');
  if(!box.children.length) WHEEL.forEach(()=>box.appendChild(el('div','tire')));
  T.tires.forEach((t,i)=>{
    const [code,name]=WHEEL[i]; const st=ST[t.s]||ST[0];
    const kind = st[1]==='ok'?'ok':st[1]==='warn'?'warn':st[1]==='bad'?'bad':'';
    const d=box.children[i]; d.className='tire '+kind;
    d.innerHTML='<div class="pos">'+code+' · '+name+'</div>'+
      (t.k==null?'<div class="p muted">—</div>':'<div class="p">'+fmtU(t.k)+'<small>'+uName()+'</small></div>')+
      '<span class="pill '+st[1]+'">'+st[0]+'</span>'+
      '<div class="meta"><span>'+(t.k==null||t.c==null?'—':t.c.toFixed(0)+' °C')+'</span><span>'+(t.b<0?'แบต —':'แบต '+t.b+'%')+'</span></div>'+
      '<div class="mac">'+(t.m?t.m:'ยังไม่ได้จับคู่')+'</div>';
  });
  // จับคู่
  const pr=$('pairRow'); pr.innerHTML='';
  T.tires.forEach((t,i)=>{
    const c = T.pair===i?'wait':(t.m?'done':'');
    pr.appendChild(el('div','wchip '+c,'<b>'+WHEEL[i][0]+'</b>'+WHEEL[i][1]+'<div class="muted small num">'+(T.pair===i?'กำลังรอสัญญาณ…':(t.m?'จับคู่แล้ว':'ว่าง'))+'</div>'));
  });
  const pn=$('pairNote');
  if(T.demo){ pn.innerHTML='<b>อยู่ในโหมด DEMO</b> จับคู่ไม่ได้ สลับเป็น REAL ด้านบนก่อน'; }
  else if(T.pair>=0){ pn.innerHTML='<b>ถือเซนเซอร์ล้อ'+WHEEL[T.pair][1]+'</b> ไว้ใกล้บอร์ด (สัญญาณแรง RSSI &gt; -60) แล้วรอ ระบบจะจดจำเองเมื่อเจอ'; }
  else { pn.innerHTML='กด <b>เริ่มจับคู่</b> แล้วทำทีละล้อตามลำดับ ถ้าไม่ได้เปลี่ยนล้อใด กดข้ามล้อนั้น'; }
  $('pairStart').disabled=T.demo||T.pair>=0;
  $('pairSkip').disabled=T.pair<0; $('pairCancel').disabled=T.pair<0;
}

function renderSettings(){
  const T=S.tp;
  document.querySelectorAll('#unitSeg button').forEach(b=>b.setAttribute('aria-pressed',String((b.dataset.v==='1')===T.bar)));
  $('unitHint').textContent='หน่วย '+uName().toUpperCase();
  const dec=T.bar?2:1;
  if(!settingsDirty && document.activeElement && !['fLo','fWa','fHi'].includes(document.activeElement.id)){
    $('fLo').value=toUnit(T.lo).toFixed(dec); $('fWa').value=toUnit(T.wa).toFixed(dec); $('fHi').value=toUnit(T.hi).toFixed(dec);
    ['fLo','fWa','fHi'].forEach(id=>$(id).step=T.bar?'0.01':'0.1');
  }
  const b=$('bands'); b.innerHTML='';
  const lo=fmtU(T.lo), wa=fmtU(T.wa), hi=fmtU(T.hi), u=uName();
  [['bad','ลมอ่อน','ต่ำกว่า '+lo+' '+u],['warn','ควรตรวจ',lo+' – '+wa+' '+u],['ok','ปกติ',wa+' – '+hi+' '+u],['bad','ลมสูง','สูงกว่า '+hi+' '+u]]
    .forEach(r=>b.appendChild(el('div','row','<span class="pill '+r[0]+'" style="min-width:84px;justify-content:center">'+r[1]+'</span><span class="num">'+r[2]+'</span>')));
}

function readFields(){
  const lo=fromUnit(parseFloat($('fLo').value)), wa=fromUnit(parseFloat($('fWa').value)), hi=fromUnit(parseFloat($('fHi').value));
  return {lo,wa,hi};
}
function validate(v){
  if([v.lo,v.wa,v.hi].some(x=>!isFinite(x))) return 'กรอกตัวเลขให้ครบทั้งสามช่อง';
  if(!(v.lo>0 && v.lo<v.wa && v.wa<v.hi && v.hi<1000)) return 'ต้องเรียงจากน้อยไปมาก: ต่ำ < เตือน < สูง';
  return '';
}

/* ---------------- หน้าระบบ ---------------- */
function renderSys(){
  const s=S.sys;
  const w=$('sysWifi'); w.innerHTML='';
  const add=(box,a,b)=>{ box.appendChild(el('dt',null,a)); box.appendChild(el('dd',null,b)); };
  add(w,'ชื่อ WiFi',s.ssid); add(w,'รหัสผ่าน',s.pw); add(w,'ที่อยู่หน้านี้','http://'+s.ip);
  add(w,'ผู้เชื่อมต่อ',s.cl+' เครื่อง'); add(w,'ปิดอัตโนมัติใน',fmtT(s.off));
  const i=$('sysInfo'); i.innerHTML='';
  add(i,'เวลาที่เปิดอยู่',fmtT(s.up)); add(i,'หน่วยความจำว่าง',(s.heap/1024).toFixed(0)+' KB'); add(i,'เฟิร์มแวร์',s.ver||'—');
}
function fmtT(sec){ sec=Math.max(0,Math.round(sec)); const m=Math.floor(sec/60), s=sec%60; return m+' นาที '+String(s).padStart(2,'0')+' วินาที'; }

/* ---------------- วนอ่านค่า ---------------- */
function renderAll(){ renderGear(); renderTpms(); renderSettings(); renderSys(); }
function syncRuns(){
  const rc=S.rc.join(',');
  if(rc!==lastRc){ lastRc=rc; loadGears(); loadRuns(selIdx); }
  if(lastSeq===null) lastSeq=S.ln.seq; else if(S.ln.seq!==lastSeq){ lastSeq=S.ln.seq; capResult(); }
}
async function refresh(){
  try{ S=await getState(); failCount=0; setConn(); renderAll(); syncRuns(); }
  catch(e){ failCount++; setConn(); }
}
function setConn(){
  const c=$('conn'), t=$('connTxt');
  c.className='conn'+(SIM?' sim':(failCount>=3?' bad':''));
  t.textContent = SIM ? 'ตัวอย่าง (ไม่ได้ต่อบอร์ด)' : (failCount>=3 ? 'ขาดการเชื่อมต่อ' : 'เชื่อมต่อบอร์ดแล้ว');
}
async function loop(){ await refresh(); setTimeout(loop, 300); }

/* ---------------- ปุ่มและแท็บ ---------------- */
const PAGES=['gear','connect','settings','system'];
function showPage(p){
  if(!PAGES.includes(p)) p='gear';
  PAGES.forEach(k=>{ $('p-'+k).classList.toggle('on',k===p); $('t-'+k).setAttribute('aria-selected',String(k===p)); });
  try{ history.replaceState(null,'','#'+p); }catch(e){}
}
function bind(){
  document.querySelectorAll('.tab').forEach(b=>b.addEventListener('click',()=>showPage(b.dataset.p)));
  window.addEventListener('hashchange',()=>showPage(location.hash.slice(1)));
  $('btnLearn').addEventListener('click',doLearn);
  $('btnCancel').addEventListener('click',async()=>{ await post('/api/learncancel'); toast('ยกเลิกรอบนี้แล้ว'); refresh(); });
  $('durSeg').addEventListener('click',e=>{ const b=e.target.closest('button'); if(!b) return; durSel=+b.dataset.v; if(S) renderGear(); });
  $('runsTbl').addEventListener('click',async e=>{ const b=e.target.closest('.xbtn'); if(!b) return; await post('/api/rundel',{i:selIdx,r:b.dataset.r}); toast('ลบรอบที่ '+(+b.dataset.r+1)+' แล้ว'); refresh(); });
  $('btnGearClear').addEventListener('click',async()=>{
    const b=$('btnGearClear');
    if(!gClrArmed){ gClrArmed=true; b.textContent='กดอีกครั้งเพื่อยืนยันการลบ'; setTimeout(()=>{ gClrArmed=false; if(S) renderRuns(); },4000); return; }
    gClrArmed=false; await post('/api/gearclear',{i:selIdx}); toast('ลบข้อมูลเกียร์ '+GN[selIdx]+' แล้ว'); refresh();
  });
  $('btnLock').addEventListener('click',async()=>{ await post('/api/lock',{on:S.lrn?'0':'1'}); toast(S.lrn?'ล็อกการเรียนรู้แล้ว':'เปิดการเรียนรู้แล้ว'); refresh(); });
  $('btnClear').addEventListener('click',async()=>{
    const b=$('btnClear');
    if(!clearArmed){ clearArmed=true; b.textContent='กดอีกครั้งเพื่อยืนยันการลบ'; setTimeout(()=>{ clearArmed=false; },4000); return; }
    clearArmed=false; await post('/api/clear'); toast('ลบค่าที่เรียนรู้แล้ว'); $('lrnRes').hidden=true; refresh();
  });
  $('modeSeg').addEventListener('click',async e=>{
    const b=e.target.closest('button'); if(!b) return;
    const demo=b.dataset.v==='1'; if(demo===S.tp.demo) return;
    await post('/api/mode',{demo:demo?'1':'0'}); toast('กำลังรีสตาร์ทบอร์ด…');
  });
  $('pairStart').addEventListener('click',async()=>{ const r=await post('/api/pair',{a:'start'}); if(!r.ok) toast('จับคู่ไม่ได้ในโหมด DEMO'); refresh(); });
  $('pairSkip').addEventListener('click',async()=>{ await post('/api/pair',{a:'skip'}); refresh(); });
  $('pairCancel').addEventListener('click',async()=>{ await post('/api/pair',{a:'cancel'}); refresh(); });
  $('unitSeg').addEventListener('click',async e=>{ const b=e.target.closest('button'); if(!b) return; settingsDirty=false; await post('/api/unit',{bar:b.dataset.v}); refresh(); });
  ['fLo','fWa','fHi'].forEach(id=>$(id).addEventListener('input',()=>{ settingsDirty=true; const e=validate(readFields()); const n=$('setErr'); n.hidden=!e; n.textContent=e; }));
  $('setSave').addEventListener('click',async()=>{
    const v=readFields(), e=validate(v); const n=$('setErr'); n.hidden=!e; n.textContent=e; if(e) return;
    const r=await post('/api/tpms',{lo:v.lo.toFixed(1),wa:v.wa.toFixed(1),hi:v.hi.toFixed(1)});
    if(r.ok){ settingsDirty=false; toast('บันทึกเกณฑ์แล้ว'); refresh(); } else toast('บอร์ดไม่รับค่า');
  });
  $('setDef').addEventListener('click',()=>{
    const d=S.tp.bar?2:1; $('fLo').value=toUnit(DEF.lo).toFixed(d); $('fWa').value=toUnit(DEF.wa).toFixed(d); $('fHi').value=toUnit(DEF.hi).toFixed(d);
    settingsDirty=true; $('setErr').hidden=true;
  });
  $('wifiOff').addEventListener('click',async()=>{ await post('/api/wifi',{on:'0'}); toast('ปิด WiFi แล้ว หน้านี้จะหลุดการเชื่อมต่อ'); });
}

buildGate(); bind();
showPage(location.hash.slice(1)||'gear');
loop();
})();
</script>
</body></html>)WEBUI";
