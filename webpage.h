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
	<th>Example Slider:</th>
	<th><input id='slider' type='range' onChange="sliderFunction('slider',this.value)" min='1' max='255' value='255'></th>
</tr><tr>
<td colspan='3'>A text spanning over all 3 columns</td>
</tr><tr>
	<th><input type='button' onclick='buttonFunction(1,this.value)' class='button' value='Button1'></th>
	<th><input type='button' onclick='buttonFunction(2,this.value)' class='button' value='Button2'></th>
</tr>
</tr><tr>
	<th>Checkbox<input type='checkbox' onChange="sliderFunction('check',this.checked)" id='check'></th>
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
            document.getElementById('check').checked=(reply.check==1?true:false);
            document.getElementById('slider').value= reply.slider;
 			var thum="";
			if (reply.temperature) {
			   thum="<p>Temperature "+reply.temperature+" &deg;C</p>Humidity: "+reply.humidity+" %</p>";
			}
            document.getElementById('temperature').innerHTML = thum;
        }
      }
    };
  }
  function buttonFunction(id,value) {
      const req = new XMLHttpRequest();
      req.open('POST','/command?id='+id);
      req.send();
      updateValues();
      if (id<100) {
      document.getElementById('command').innerHTML = 'Button: '+id; }
    return false;
  }
  function sliderFunction(name,val) {
      console.log(name+'='+val);
      const req = new XMLHttpRequest();
      req.open('POST','/slider?'+name+'='+val);
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
