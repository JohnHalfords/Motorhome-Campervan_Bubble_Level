const char wwwroot[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no'/>
    <title>Bubble Level</title>
    <style>
      html,body{
        margin:0;
        height:100%;
        background:#020617;
        color:#e5e7eb;
        font-family:system-ui
      }
      #app{
        height:100%;
        display:flex;
        flex-direction:column;
        align-items:center;
        justify-content:center
      }
      #status{
        padding:6px 14px;
        border-radius:999px;
        font-size:14px;
        margin-bottom:12px;
        display:flex;
        align-items:center;
        gap:6px
      }
      .connected{
        background:#064e3b;
        color:#6ee7b7
      }
      .disconnected{
        background:#7f1d1d;
        color:#fecaca
      }
      #spinner{
        width:14px;
        height:14px;
        animation:spin 1s linear infinite;
      }
      @keyframes spin{
        to{
          transform:rotate(360deg)
          }
        }
      #bubbleBox{
        width:260px;
        height:260px;
        border-radius:50%;
        border:2px solid #334155;
        position:relative
      }
      #bubble{
        width:40px;
        height:40px;
        border-radius:50%;
        background:#22c55e;
        position:absolute;
        left:50%;
        top:50%;
        transform:translate(-50%,-50%)
      }
      #values{
        margin-top:12px;
        font-family:monospace;
        display:flex;
        gap:12px
      }
      .val{
        width:90px;
        text-align:center;
      }
    </style>
  </head>

  <body>
    <div id='app'>
      <div style="text-align: center;">
        <h3>Motorhome/Campervan</h3>
        <h2>Bubble Level</h2>
      </div>

      <div id='status' class='disconnected'>
        <svg id='spinner' viewBox='0 0 24 24' stroke='#94a3b8' stroke-width='2' fill='none'>
          <line x1='12' y1='2' x2='12' y2='22'/>
          <line x1='2' y1='12' x2='22' y2='12'/>
        </svg>
        <span id='statusText'>no connection</span>
      </div>

      <div id='bubbleBox'>
        <svg viewBox='0 0 100 100' style='position:absolute;inset:0'>
          <circle cx='50' cy='50' r='45' fill='none' stroke='#334155' stroke-width='0.5'/>
          <circle cx='50' cy='50' r='37.5' fill='none' stroke='#334155' stroke-width='0.5'/>
          <circle cx='50' cy='50' r='30' fill='none' stroke='#334155' stroke-width='0.5'/>
          <circle cx='50' cy='50' r='22.5' fill='none' stroke='#334155' stroke-width='0.5'/>
          <line x1='50' y1='5' x2='50' y2='95' stroke='#334155' stroke-width='0.5'/>
          <line x1='5' y1='50' x2='95' y2='50' stroke='#334155' stroke-width='0.5'/>
        </svg>
        <div id='bubble'></div>
      </div>

      <div id='values'>
        <div class='val'>X:<span id='x'>0.0</span></div>
        <div class='val'>Y:<span id='y'>0.0</span></div>
        <div class='val'>D:<span id='d'>0.0</span></div>
      </div>

    </div>

    <script>
      const WS_URL='@@WS_URL@@'
      let lastData=Date.now()

      let ws
        const statusEl=document.getElementById('status')
        const statusText=document.getElementById('statusText')
        const spinner=document.getElementById('spinner')
        const bubble=document.getElementById('bubble')

      function connect(){
        ws=new WebSocket(WS_URL)

        ws.onopen=()=>{
          setDisconnected()
          statusEl.className='connected'
          statusText.textContent='connected - live data'
        }

        ws.onclose=()=>{
          setDisconnected()
          setTimeout(connect,1000)
        }
        ws.onmessage = e => {
          lastData = Date.now()
          setConnected()

          const j = JSON.parse(e.data)
          const x = j.x
          const y = j.y
          const d = Math.hypot(x, y)

          document.getElementById('x').textContent = x.toFixed(1)
          document.getElementById('y').textContent = y.toFixed(1)
          document.getElementById('d').textContent = d.toFixed(1)

          bubble.style.transform = `translate(-50%, -50%) translate(${x}px, ${y}px)`
          bubble.style.background = bubbleColor(d)
        }

      }
      function bubbleColor(dist){
        if(dist < 2) return '#22c55e'   // groen
        if(dist < 6) return '#facc15'   // oranje
        return '#ef4444'                // rood
      }
      function setConnected(){
        statusEl.className='connected'
        statusText.textContent='connected - live data'
        spinnerRun()
      }
      function setDisconnected(){
        statusEl.className='disconnected'
        statusText.textContent='no connection'
        spinnerStop()
      }
      function spinnerRun(){
       spinner.style.animationPlayState = 'running'
      }
      function spinnerStop(){
        spinner.style.animationPlayState = 'paused'
      }
      setInterval(()=>{
        if(Date.now() - lastData > 500){
          setDisconnected()
          if(ws && ws.readyState === WebSocket.OPEN){
            ws.close()   // ⬅ forceer close → snelle reconnect
          }
        }
      },200)

      connect()
    </script>
  </body>
</html>
)rawliteral";