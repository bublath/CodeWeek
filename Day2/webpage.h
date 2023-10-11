const char * html_page = R""""(
<html>
<head>
<style>
.button {
  display: inline-block;
  padding: 10px 25px;
  cursor: pointer;
  text-align: center;
  text-decoration: none;
  outline: none;
  color: #fff;
  width: 150px;
  background-color: #00aaff;
  border: none;
  border-radius: 15px;
  box-shadow: 0 3px #999;
}
.button2 {
	background-color:#00bb00;
}
.button:active {
  background-color: #22222;
  box-shadow: 0 3px #666;
  transform: translateY(3px);
}
</style>
</head>
<body onload='init()'>
<p><h1>ESP LED Driver</h1></p>
<p><h2 id='temperature'></h2></p>
<p><h2 id='command'></h2></p>
<table>
<tr>
 	<th>Color rotation<input type='checkbox' onChange="sliderFunction('rgb',this.checked)" id='rgb'></th>
  <th>Color <input type='color' onChange="sliderFunction('color',this.value)" id='color'></th>
<tr>
	<th>Speed</th>
  <th>Fade</th>
</tr><tr>
	<th><input id='speed' type='range' onChange="sliderFunction('speed',this.value)" min='1' max='255' value='255'></th>
  <th><input id='fade' type='range' onChange="sliderFunction('fade',this.value)" min='1' max='255' value='255'></th>
</tr><tr>
	<th><input type='button' onclick='buttonFunction(1,this.value)' class='button' value='Rainbow'></th>
	<th><input type='button' onclick='buttonFunction(2,this.value)' class='button' value='KITT'></th>
</tr><tr> 
  <th><input type='button' onclick='buttonFunction(0,this.value)' class='button' value='OFF'></th>
  <th><input type='button' onclick='buttonFunction(100,this.value)' class='button' value='Save'></th>
</tr>
<script>
  function init() {
	  updateValues();
	  var id=setInterval(updateValues,5000);
  }
  function updateValues() {
    const req = new XMLHttpRequest();
    req.open('GET','/settings');
    req.send();
    req.onreadystatechange = () => {
      if (req.readyState === XMLHttpRequest.DONE) {
        const status = req.status;
        if (status == 200) {
            var reply = JSON.parse(req.responseText);
            document.getElementById('rgb').checked=(reply.rgb==1?true:false);
            document.getElementById('color').value= reply.color;
            console.log(reply.color);
            document.getElementById('speed').value= reply.speed;
            document.getElementById('fade').value= reply.fade; 			
            var thum="";
            if (reply.temperature) {
              thum="<p>Temperature "+reply.temperature+" &deg;C</p>Humidity: "+reply.humidity+" %</p>";
            }
            document.getElementById('temperature').innerHTML = thum;
            document.getElementById('command').innerHTML = "Current Effect: "+reply.effect; 
        }
      }
    };
  }
  function buttonFunction(id,value) {
      const req = new XMLHttpRequest();
      req.open('POST','/command?id='+id);
      req.send();
      updateValues();
    return false;
  }
  function sliderFunction(name,val) {
      console.log(name+'='+val);
      const req = new XMLHttpRequest();
      req.open('POST','/slider?'+name+'='+encodeURIComponent(val));
      req.send();
      return false;
  }
</script>
</body>
</html>
)"""";

const char * login_page = R""""(
<html>
<head>
<style>
.button:active {
  transform: translateY(3px);
}
</style>
</head>
<body>
<p><h1>ESP LED Login Page</h1></p>
<p>SSID:<input type='text' size=25 id='ssid'></p>
<p>Password:<input type='text' size=25 id='pw'></p>
<p><input type='button' onclick='submit()' value='Submit'></p>
<script>
function submit() {
	const req = new XMLHttpRequest();
	var ssid=document.getElementById('ssid').value;
	var pw=document.getElementById('pw').value;
	var uri="ssid="+encodeURIComponent(ssid)+"&pw="+encodeURIComponent(pw);
    req.open('POST',"/login?"+uri);
    req.send();
    return false;
}
</script>
)"""";
