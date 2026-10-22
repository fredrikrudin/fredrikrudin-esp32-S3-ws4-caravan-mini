// esp32-s3-ws4-caravan-mini v1.0
#pragma once
/* The web page: one fixed HTML file that fetches /json every 30 seconds and draws
 * itself in the browser. The board only builds a few kB of JSON per update, and
 * the page adapts to any screen: one column on a phone, more on a tablet or PC.
 * Sections appear only for what the board has (Victron devices, RuuviTags ...). */

static const char WEB_PAGE[] PROGMEM = R"PAGE(<!DOCTYPE html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#111418">
<link rel="icon" href="data:,">
<title>Display</title>
<style>
:root{--bg:#111418;--card:#1b1f25;--card2:#242a32;--line:#2e353f;--text:#e6e8eb;--dim:#9aa3ad;
--blue:#38a9f5;--solar:#f5b32b;--red:#e74c3c;--green:#2ecc71;--amber:#f39c12;--load:#e8604c}
*{box-sizing:border-box}
html{-webkit-text-size-adjust:100%}
body{margin:0;background:var(--bg);color:var(--text);font:15px/1.4 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
header{display:flex;align-items:center;gap:12px;flex-wrap:wrap;padding:14px 16px 6px;max-width:1240px;margin:0 auto}
header h1{font-size:20px;margin:0;flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.clock{text-align:right;line-height:1.1;margin-left:auto}.clock b{font-size:26px;font-variant-numeric:tabular-nums}
.clock div{color:var(--dim);font-size:12px}
.upd{font:inherit;font-size:12px;padding:3px 10px;border-radius:99px;border:1px solid var(--line);background:var(--card2);color:var(--text);cursor:pointer;white-space:nowrap}
.pill{font-size:12px;padding:3px 9px;border-radius:99px;background:var(--card2);color:var(--dim);white-space:nowrap}
.pill.ok::before{content:"";display:inline-block;width:7px;height:7px;border-radius:50%;background:var(--green);margin-right:6px;vertical-align:1px}
.pill.bad{background:#5a3b10;color:#ffd08a}
main{max-width:1240px;margin:0 auto;padding:6px 16px 24px}
.grid{display:grid;gap:12px;grid-template-columns:repeat(auto-fill,minmax(min(100%,310px),1fr));grid-auto-flow:row dense;align-items:start}
.card{background:var(--card);border-radius:14px;padding:14px 16px;min-width:0}
.wide{grid-column:1/-1}
h2{font-size:13px;font-weight:600;letter-spacing:.04em;text-transform:uppercase;color:var(--dim);margin:0 0 10px}
.alarm{padding:11px 14px;border-radius:12px;margin:0 0 10px;font-weight:600}
.lvl2{background:#9f2a1e}.lvl1{background:#8a5a0b}
.big{font-size:44px;font-weight:700;line-height:1;font-variant-numeric:tabular-nums}
.mid{font-size:24px;font-weight:600;font-variant-numeric:tabular-nums}
.sub{color:var(--dim);font-size:13px}
.bar{height:10px;border-radius:5px;background:var(--card2);overflow:hidden;margin:10px 0 8px}
.bar i{display:block;height:100%;border-radius:5px}
.flex{display:flex;justify-content:space-between;align-items:baseline;gap:10px;flex-wrap:wrap}
.tiles{display:grid;gap:8px;grid-template-columns:repeat(auto-fill,minmax(104px,1fr))}
.tile{background:var(--card2);border-radius:10px;padding:8px 10px;min-width:0}
.tile .sub{white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.row{display:flex;justify-content:space-between;align-items:center;gap:10px;padding:9px 0;border-top:1px solid var(--line)}
.row:first-of-type{border-top:0}
.row>div:first-child{min-width:0}.row .r{text-align:right;flex-shrink:0}
.name{overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.chg{color:var(--solar);font-weight:600}
svg{display:block;width:100%}
svg.c24{height:170px}svg.sp{height:46px;margin:4px 0}
.axis{display:flex;justify-content:space-between;font-size:11px;color:var(--dim);margin-top:3px}
.legend{display:flex;gap:14px;flex-wrap:wrap;font-size:12px;color:var(--dim);margin-top:6px}
.legend i{display:inline-block;width:10px;height:10px;border-radius:2px;margin-right:5px;vertical-align:-1px}
.fc{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:10px}
.fc>div{background:var(--card2);border-radius:10px;padding:8px;text-align:center;font-size:13px}
/* ---- Power: chargers | battery | AC side, DC loads under the battery, like the display ---- */
.pw{display:grid;grid-template-columns:minmax(0,1fr) 40px minmax(0,1.25fr) 40px minmax(0,1fr);
 grid-template-areas:"src l1 bat l2 ac" ". . v . ." ". . loads . .";--c-solar:#f39c12;--c-dcdc:#16a085;--c-ac:#4a5a6a;--c-inv:#8e5bd0;--c-batt:#2f86d1;--c-loads:#27ae60}
.pw .col{display:flex;flex-direction:column;gap:12px;justify-content:center;min-width:0}
.pw .src{grid-area:src}.pw .ac{grid-area:ac}.pw .bat{grid-area:bat}.pw .loads{grid-area:loads}.pw .v{grid-area:v;height:26px}
.pw .l1{grid-area:l1}.pw .l2{grid-area:l2}.pw .l1,.pw .l2{display:flex;flex-direction:column}
.pw .links{flex:1;display:flex;flex-direction:column;gap:12px}.pw .links>div{flex:1;display:flex;align-items:center}
.pw .l1>.vln{display:none}
.box{border-radius:12px;overflow:hidden;background:var(--dev);color:#fff;position:relative;min-width:0}
.box .t{background:rgba(0,0,0,.22);padding:5px 10px;font-size:13px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
.box .b{padding:6px 10px 10px;text-align:center;position:relative;z-index:1}
.box .val{font-size:32px;font-weight:600;line-height:1.1;font-variant-numeric:tabular-nums}
.box .st{font-size:14px}.box .d{font-size:12.5px;opacity:.8;font-variant-numeric:tabular-nums;min-height:1em}
.box.idle{--dev:#2a3139!important;color:var(--dim)}.box.alm{--dev:#b03a2e!important}
.bat .box{flex:1;display:flex;flex-direction:column}.bat .box .b{flex:1;display:flex;flex-direction:column;justify-content:center}
.bat .box .val{font-size:52px}
.box .soc{height:8px;margin:6px 4px 8px;border-radius:4px;background:rgba(0,0,0,.25);overflow:hidden}
.box .soc i{display:block;height:100%;background:#fff;opacity:.85}
.box .two{display:flex;justify-content:center;gap:18px;flex-wrap:wrap;font-size:14px;font-variant-numeric:tabular-nums}
.box svg.bg{position:absolute;left:0;bottom:0;width:100%;height:26%;opacity:.22;pointer-events:none}
.ln{width:100%;height:6px;border-radius:3px;background:var(--line)}
.vln{width:6px;height:100%;margin:0 auto;border-radius:3px;background:var(--line)}
.ln.on{background:repeating-linear-gradient(90deg,var(--dev) 0 12px,transparent 12px 20px),color-mix(in srgb,var(--dev) 35%,transparent);background-size:20px 100%,100% 100%;animation:fx 1s linear infinite}
.ln.rev{animation-direction:reverse}
.vln.on{background:repeating-linear-gradient(180deg,var(--dev) 0 12px,transparent 12px 20px),color-mix(in srgb,var(--dev) 35%,transparent);background-size:100% 20px,100% 100%;animation:fy 1s linear infinite}
@keyframes fx{to{background-position:20px 0,0 0}}@keyframes fy{to{background-position:0 20px,0 0}}
@media (prefers-reduced-motion:reduce){.ln.on,.vln.on{animation:none}}
@media (max-width:640px){
 .pw{grid-template-columns:minmax(0,1fr);grid-template-areas:"src" "ac" "l1" "bat" "v" "loads";row-gap:10px}
 .pw .src,.pw .ac{display:grid;grid-template-columns:1fr 1fr;gap:10px}
 .pw .l2,.pw .l1 .links{display:none}.pw .l1{height:26px}.pw .l1>.vln{display:block}.pw .v{height:16px}
 .box .val{font-size:26px}.bat .box .val{font-size:46px}}
/* Weather and temperatures side by side in one card */
.wt{display:grid;grid-template-columns:minmax(0,1fr) minmax(0,1.3fr);gap:14px 28px}
@media (max-width:700px){.wt{grid-template-columns:minmax(0,1fr)}}
footer{max-width:1240px;margin:0 auto;padding:0 16px 28px}
/* the About card: like the others, but quieter */
.about{border:1px solid var(--line);border-radius:14px;padding:12px 16px;color:var(--dim);font-size:12px;line-height:1.6}
.about h2{font-size:11px;margin-bottom:8px}
.about dl{display:grid;grid-template-columns:max-content 1fr;gap:4px 14px;margin:0}
.about dt{color:var(--dim);opacity:.75}
.about dd{margin:0;min-width:0;overflow-wrap:anywhere}
.about a{color:var(--dim);text-decoration:underline;text-decoration-color:var(--line);text-underline-offset:2px}
.about a:hover{color:var(--blue)}
@media (max-width:420px){.about dl{grid-template-columns:1fr}.about dt{margin-top:6px}}
a{color:var(--blue)}
.empty{color:var(--dim);font-size:13px}
/* while the first answer is on its way: grey placeholder cards, or the last values dimmed */
.skel h2,.skel i{display:block;border-radius:7px;background:linear-gradient(90deg,var(--card2) 0%,#2f3640 50%,var(--card2) 100%);background-size:200% 100%;animation:sk 1.4s linear infinite}
.skel h2{width:40%;height:12px}.skel i{height:16px;margin:12px 0 0}
@keyframes sk{from{background-position:200% 0}to{background-position:-200% 0}}
@media (prefers-reduced-motion:reduce){.skel h2,.skel i{animation:none}}
body.stale main{opacity:.55}
main{transition:opacity .3s}
@media (max-width:420px){.big{font-size:38px}header h1{font-size:18px;flex-basis:100%}.clock b{font-size:22px}main{padding:6px 10px 20px}}
</style></head><body>
<header><h1 id="name">&nbsp;</h1><span class="pill" id="state">Connecting&hellip;</span><button class="upd" id="upd">&#8635;</button>
<div class="clock"><b id="time">--:--</b><div id="date"></div></div></header>
<main><div id="alarms"></div><div class="grid" id="grid">
<section class="card skel"><h2></h2><i style="width:55%;height:34px"></i><i></i></section>
<section class="card skel"><h2></h2><i style="width:45%;height:34px"></i><i style="width:80%"></i></section>
<section class="card skel"><h2></h2><i></i><i style="width:70%"></i><i style="width:85%"></i></section>
<section class="card skel"><h2></h2><i style="width:60%"></i><i></i><i style="width:75%"></i></section>
</div></main>
<footer id="foot"></footer>
<script>
"use strict";
const $=id=>document.getElementById(id);
const esc=s=>String(s==null?"":s).replace(/[&<>"']/g,c=>({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"})[c]);
const num=(v,d=0)=>v==null||isNaN(v)?"--":(+v).toFixed(d);
const card=(t,body,cls="")=>`<section class="card ${cls}"><h2>${t}</h2>${body}</section>`;
/* Texts follow the display's language: /lang gives the translations of the page's
   texts: every T(...) call below with a quoted text (tools/i18n.py collects them into
   lang/<code>.po, web.cpp finds them for /lang). Each %s is filled in from the
   arguments after the text, in order. */
let TXT={},lang="en",langV="";
const T=(t,...a)=>{let i=0;return (TXT[t]||t).replace(/%s/g,()=>a[i++])};
async function setLang(l,v){
  if(l==lang&&v==langV)return;
  try{const r=await fetch("/lang",{cache:"no-store"});if(r.ok){TXT=await r.json();lang=l;langV=v;document.documentElement.lang=l;keep("lang",{l,v,TXT})}}catch(e){}
}
/* The last answer is kept in this browser, so the page can show it straight away
   (dimmed) while the board answers. Nothing breaks if the browser keeps nothing. */
const keep=(k,v)=>{try{localStorage.setItem("ws4."+k,JSON.stringify(v))}catch(e){}};
const kept=k=>{try{return JSON.parse(localStorage.getItem("ws4."+k))}catch(e){return null}};
let timer=0,busy=false,fails=0,lastOk=0,data=null;

function schedule(ms){clearTimeout(timer);timer=setTimeout(load,ms)}
/* Every 30 s, and every 2 minutes once the page has been left alone for 2 minutes:
   each update is work for the board. Update or coming back to the tab
   makes it quick again. */
let active=Date.now();
const every=()=>Date.now()-active<120000?30000:120000;
function wake(){active=Date.now();schedule(0)}
async function load(){
  if(busy)return;
  if(document.hidden){schedule(10000);return}   // nobody is looking: don't load the board
  busy=true;
  const ac=new AbortController(),to=setTimeout(()=>ac.abort(),8000);
  try{
    const r=await fetch("/json",{signal:ac.signal,cache:"no-store"});
    if(r.status==401){location.href="/login";return}
    if(!r.ok)throw new Error(r.status);
    data=await r.json();fails=0;lastOk=Date.now();
    if(data.lang)await setLang(data.lang,data.version);
    render(data);document.body.classList.remove("stale");
    keep("data",{t:lastOk,d:data});
  }catch(e){fails++}
  finally{clearTimeout(to);busy=false;status();schedule(fails?Math.min(30000,4000*fails):every())}
}
function status(){
  const s=$("state");
  $("upd").textContent="↻ "+T("Update");
  if(!lastOk){s.className="pill"+(fails?" bad":"");s.textContent=fails?T("No answer, retrying"):T("Connecting…");return}
  const age=Math.round((Date.now()-lastOk)/1000);
  if(fails){s.className="pill bad";s.textContent=T("Offline · data %s old",age<90?age+" s":Math.round(age/60)+" min")}
  else{s.className="pill ok";s.textContent=T("Connected")}
}
document.addEventListener("visibilitychange",()=>{if(!document.hidden)wake()});
$("upd").addEventListener("click",wake);

/* ---- small charts as SVG ---- */
function bars24(h){
  const n=h.solar.length,W=480,H=150,B=H-20,bw=Math.max(3,Math.floor(W/n/2)-2);
  let max=1;for(let i=0;i<n;i++)max=Math.max(max,h.solar[i]||0,h.load[i]||0);
  let s=`<svg class="c24" viewBox="0 0 ${W} ${B}" preserveAspectRatio="none" role="img" aria-label="${T("Last 24 hours")}"><line x1="0" y1="${B}" x2="${W}" y2="${B}" stroke="#2e353f" vector-effect="non-scaling-stroke"/>`;
  const step=W/n;
  for(let i=0;i<n;i++){
    const x=i*step+step/2-bw;
    const a=(h.solar[i]||0)/max*(B-10),b=(h.load[i]||0)/max*(B-10);
    if(a>0)s+=`<rect x="${x.toFixed(1)}" y="${(B-a).toFixed(1)}" width="${bw}" height="${a.toFixed(1)}" rx="1" fill="var(--solar)"/>`;
    if(b>0)s+=`<rect x="${(x+bw+1).toFixed(1)}" y="${(B-b).toFixed(1)}" width="${bw}" height="${b.toFixed(1)}" rx="1" fill="var(--load)"/>`;
  }
  let p="";for(let i=0;i<n;i++)if(h.soc[i]!=null)p+=`${(i*step+step/2).toFixed(1)},${(B-h.soc[i]/100*(B-10)).toFixed(1)} `;
  if(p)s+=`<polyline points="${p}" fill="none" stroke="var(--blue)" stroke-width="2.5" stroke-linejoin="round" vector-effect="non-scaling-stroke"/>`;
  s+=`</svg><div class="axis"><span>-${n-1}h</span><span>-${Math.round((n-1)/2)}h</span><span>${T("now")}</span></div>`;
  return s+`<div class="legend"><span><i style="background:var(--solar)"></i>${T("Solar")}</span><span><i style="background:var(--load)"></i>${T("Consumption")}</span><span><i style="background:var(--blue)"></i>${T("Battery (%)")}</span><span>${T("Max %s Wh per hour",Math.round(max))}</span></div>`;
}
function spark(v,color,label,unit){
  const pts=v.map((x,i)=>[i,x]).filter(p=>p[1]!=null);
  if(pts.length<2)return "";
  let mn=Infinity,mx=-Infinity;pts.forEach(p=>{mn=Math.min(mn,p[1]);mx=Math.max(mx,p[1])});
  if(mx-mn<1)mx=mn+1;
  const W=240,H=56,n=v.length-1;
  const pl=pts.map(p=>`${(p[0]/n*(W-4)+2).toFixed(1)},${(H-4-(p[1]-mn)/(mx-mn)*(H-8)).toFixed(1)}`).join(" ");
  return `<div class="tile"><div class="sub">${label}</div><svg class="sp" viewBox="0 0 ${W} ${H}" preserveAspectRatio="none"><polyline points="${pl}" fill="none" stroke="${color}" stroke-width="2" vector-effect="non-scaling-stroke"/></svg><div class="sub">${Math.round(mn)}–${Math.round(mx)} ${unit}</div></div>`;
}
/* ---- Power: the display's Power tab. Chargers on the left, the AC side on the
   right, DC loads under the battery; a line lights up and moves while energy flows ---- */
const PCOL={solar:"var(--c-solar)",dcdc:"var(--c-dcdc)",ac:"var(--c-ac)",inv:"var(--c-inv)"};
function bgSpark(v){
  const p=(v||[]).map((y,i)=>[i,y]).filter(q=>q[1]!=null);if(p.length<2)return"";
  let mn=Math.min(...p.map(q=>q[1])),mx=Math.max(...p.map(q=>q[1]));if(mx-mn<1)mx=mn+1;const n=v.length-1;
  return `<svg class="bg" viewBox="0 0 100 30" preserveAspectRatio="none"><polyline fill="none" stroke="#fff" stroke-width="2" vector-effect="non-scaling-stroke" points="${p.map(q=>`${(q[0]/n*100).toFixed(1)},${(28-(q[1]-mn)/(mx-mn)*26).toFixed(1)}`).join(" ")}"/></svg>`;
}
function pBox(v,sp){
  let val="--",det="";
  if(v.ok){
    if(v.kind=="dcdc"){val=v.out_v!=null?num(v.out_v,2)+" V":"--";if(v.in_v!=null)det=T("In %s V",num(v.in_v,2))}
    else if(v.kind=="inv"){val=v.power!=null?num(v.power,0)+" VA":"--";if(v.ac_v!=null)det=num(v.ac_v,0)+" V"+(v.ac_i!=null?"  "+num(v.ac_i,1)+" A":"")}
    else{val=v.power!=null?num(v.power,0)+" W":"--";
      if(v.kind=="solar"&&v.yield_kwh!=null)det=T("Today %s kWh",num(v.yield_kwh,2));
      else if(v.kind=="ac"&&v.v!=null)det=num(v.v,2)+" V  "+num(v.a,1)+" A"}
  }
  const cls=v.alarm?" alm":v.ok&&v.flow?"":" idle";
  return `<div class="box${cls}" style="--dev:${PCOL[v.kind]}"><div class="t">${esc(v.name)}</div><div class="b"><div class="val">${val}</div><div class="st">${esc(v.ok?v.state||"":v.status)}</div><div class="d">${esc(det)}</div></div>${v.ok&&v.flow&&sp?bgSpark(sp):""}</div>`;
}
const pLine=(v,rev)=>`<div><div class="ln${v.ok&&v.flow?" on":""}${rev?" rev":""}" style="--dev:${PCOL[v.kind]}"></div></div>`;
function powerCard(d){
  const vs=(d.victron||[]).filter(v=>v.kind&&v.kind!="other");
  if(!vs.length)return `<div class="empty">${T("No Victron devices added")}</div>`;
  const rec=(d.history&&d.history.recent)||{};
  const left=vs.filter(v=>v.kind=="solar"||v.kind=="dcdc"),right=vs.filter(v=>v.kind=="ac"||v.kind=="inv");
  const bm=vs.find(v=>v.kind=="batt"),b=d.battery||{};
  let bat;
  if(bm&&bm.ok&&b.soc!=null){
    const extra=b.current!=null&&b.current>0.05?T("Charging"):b.ttg_min!=null&&b.ttg_min>=0&&b.current<-0.05?T("%s left",Math.floor(b.ttg_min/60)+"h "+String(b.ttg_min%60).padStart(2,"0")+"m"):"";
    bat=`<div class="val">${num(b.soc,0)}%</div><div class="soc"><i style="width:${Math.max(0,Math.min(100,b.soc))}%${b.soc<20?";background:#ff8a7a":""}"></i></div>
<div class="two"><span>${num(b.voltage,2)} V</span><span>${b.current>0?"+":""}${num(b.current,1)} A</span></div>
<div class="two d"><span>${b.power>0?"+":""}${num(b.power,0)} W</span>${extra?`<span>${extra}</span>`:""}${bm.temp!=null?`<span>${num(bm.temp,0)}°C</span>`:""}</div>`;
  }else if(bm){bat=`<div class="val">--</div><div class="st">${esc(bm.status)}</div>`}
  else{const src=vs.find(v=>v.ok&&v.v!=null);bat=`<div class="val">--</div><div class="st">${src?num(src.v,2)+" V":""}</div><div class="d">${T("No battery monitor")}</div>`}
  const anyIn=left.concat(right.filter(v=>v.kind=="ac")).some(v=>v.ok&&v.flow);
  const L=d.loads;
  return `<div class="pw">
<div class="col src">${left.map((v,i)=>pBox(v,v.kind=="solar"&&i==0?rec.solar:null)).join("")}</div>
<div class="l1"><div class="links">${left.map(v=>pLine(v)).join("")}</div><div class="vln${anyIn?" on":""}" style="--dev:var(--c-solar)"></div></div>
<div class="col bat"><div class="box${bm&&bm.ok?"":" idle"}" style="--dev:var(--c-batt)"><div class="t">${esc(bm?bm.name:T("Battery"))}</div><div class="b">${bat}</div></div></div>
<div class="l2"><div class="links">${right.map(v=>pLine(v,v.kind=="ac")).join("")}</div></div>
<div class="col ac">${right.map(v=>pBox(v)).join("")}</div>
${L?`<div class="v"><div class="vln${L.power>2?" on":""}" style="--dev:var(--c-loads)"></div></div>
<div class="col loads"><div class="box" style="--dev:var(--c-loads)"><div class="t">${T("DC loads")}</div><div class="b"><div class="val">${num(L.power,0)} W</div><div class="d">${esc(L.note)}</div></div>${bgSpark(rec.load)}</div></div>`:""}
</div>`;
}
/* ---- the page ---- */
function render(d){
  document.title=d.name||T("Display");$("name").textContent=d.name||"";
  $("time").textContent=d.time||"--:--";$("date").textContent=d.date||"";
  $("alarms").innerHTML=(d.alarms||[]).map(a=>`<div class="alarm lvl${a.level}">⚠ ${esc(a.text)}</div>`).join("");
  const c=[];

  /* power: the display's Power tab */
  c.push(card(T("Power"),powerCard(d),"wide"));

  /* weather and temperatures, together right under Power */
  const tt=(d.temperatures||[]).map(x=>`<div class="tile"><div class="sub">${esc(x.name)}</div><span class="mid">${num(x.temperature,1)}</span><span class="sub">°C</span></div>`);
  const w=d.weather||{};
  let wb;
  if(w.temperature!=null){
    wb=`<div class="flex"><span><span class="big">${num(w.temperature,1)}°</span> <span class="sub">${esc(w.condition)}</span></span><span class="sub">${T("H %s° · L %s°",num(w.today_max),num(w.today_min))}</span></div><div class="sub" style="margin-top:6px">${T("Feels %s°",num(w.feels_like,1))} · ${T("humidity %s",num(w.humidity)+"%")} · ${T("wind %s m/s",num(w.wind,1))}</div>`;
    if(w.forecast)wb+=`<div class="fc">${w.forecast.map(f=>`<div><b>${esc(f.day)}</b><div class="sub">${esc(f.condition)}</div>${num(f.max)}° / ${num(f.min)}°</div>`).join("")}</div>`;
  }else wb=`<div class="empty">${d.has_location===false?T("No location set"):T("Loading weather…")}</div>`;
  c.push(tt.length?`<section class="card wide"><div class="wt"><div><h2>${T("Temperatures")}</h2><div class="tiles">${tt.join("")}</div></div><div><h2>${T("Weather")}</h2>${wb}</div></div></section>`:card(T("Weather"),wb,"wide"));

  /* history */
  const h=d.history;
  if(h&&h.hours&&(h.hours.solar.some(x=>x>0)||h.hours.load.some(x=>x>0)))
    c.push(card(T("Last 24 hours"),bars24(h.hours),"wide"));
  if(h&&h.recent){
    const sl=[spark(h.recent.solar,"var(--solar)",T("Solar"),"W"),spark(h.recent.soc,"var(--blue)",T("Battery"),"%"),spark(h.recent.load,"var(--load)",T("Consumption"),"W")].filter(x=>x);
    // full width only with two or three charts; a single one fits in a normal card
    if(sl.length)c.push(card(T("Last 10 minutes"),`<div class="tiles" style="grid-template-columns:repeat(auto-fill,minmax(180px,1fr))">${sl.join("")}</div>`,sl.length>1?"wide":""));
  }

  /* the board's own battery */
  const bb=d.board_battery;
  if(bb)c.push(card(T("Board battery"),`<div class="flex"><span class="mid">${num(bb.percent)}%</span><span class="sub">${num(bb.voltage,2)} V · ${bb.charging?T("charging or full"):bb.on_battery?T("running on battery"):T("on external power")}</span></div><div class="bar"><i style="width:${bb.percent}%;background:var(--green)"></i></div>`));

  $("grid").innerHTML=c.join("");
  const rate=every()<60000?T("every 30 s"):T("every 2 minutes");
  const links=`<a href="/log">${T("log")}</a> · <a href="/files">${T("files")}</a> · <a href="/json">JSON</a>${d.logout?` · <a href="/logout">${T("Log out")}</a>`:""}`;
  $("foot").innerHTML=`<section class="about"><h2>${T("About")}</h2><dl>
<dt>${T("Updates")}</dt><dd>${T("Updated %s",new Date().toLocaleTimeString(lang))} · ${rate}<br><span style="opacity:.75">${T("Every 30 s while the page is in use, every 2 minutes after 2 minutes without use. Update fetches at once.")}</span></dd>
<dt>${T("Board")}</dt><dd>${esc(d.project)} v${esc(d.version)} · ${T("up %s",fmtUp(d.uptime_min))} · ${links}</dd>
<dt>${T("Source")}</dt><dd><a href="https://github.com/fredrikrudin/${esc(d.project)}">github.com/fredrikrudin/${esc(d.project)}</a></dd>
<dt>${T("License")}</dt><dd>© Fredrik Rudin · <a href="https://creativecommons.org/licenses/by-nc/4.0/">CC BY-NC 4.0</a> · ${T("Written with the help of Claude (Anthropic)")}</dd>
</dl></section>`;
}
function fmtUp(m){if(m==null)return"--";if(m<60)return m+" min";if(m<2880)return Math.floor(m/60)+" h "+(m%60)+" min";return T("%s days",Math.floor(m/1440))}

/* start: the last values (if this browser has them) while the board answers */
{
  const kl=kept("lang");
  if(kl&&kl.l){TXT=kl.TXT||{};lang=kl.l;langV=kl.v||"";document.documentElement.lang=lang}
  const kd=kept("data");
  if(kd&&kd.d)try{
    render(kd.d);document.body.classList.add("stale");
    $("state").textContent=T("Values from %s, loading…",new Date(kd.t).toLocaleTimeString(lang,{hour:"2-digit",minute:"2-digit"}));
  }catch(e){$("grid").innerHTML=""}
  $("upd").textContent="↻ "+T("Update");
}
load();
</script></body></html>
)PAGE";
