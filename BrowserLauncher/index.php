<?php
	session_start();
	require_once('../https_redir.php');
	require_once('../auth.php'); 

	// make login command
	$username = $_SESSION['username'];
	$password = '0x'.md5($_SESSION['password']);
	$cmd      = "noLoginPass";

	if(isset($username) && isset($password))
		$cmd = "-login $username -pwd $password";
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<HTML>
<HEAD>
<TITLE>War Inc. Battle Zone launcher</TITLE>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta name="description" content="A perfect game">
</HEAD>
<BODY>

<div style="width:640px;">
	<div style="width:640px; text-align:center; font-size:34px; color:Red;">War Inc. Battle Zone: Game Launching!</div><br>
	The War Inc Updater should be starting your game soon. If you need to, follow the instructions to install the War Inc Launcher Plugin.<br><br>

	<div id="welcome" style="display:none">
		<img src="welcomeback.jpg"><br>
	</div>
	
	<div id="ieApplet" style="display:none">
		<img src="ie1.png"><br>
	</div>

	<div id="ffApplet" style="display:none">
		<img src="ff1.png"><br>
		<img src="ff2.png"><br>
		<img src="ff3.png"><br>
		<img src="ff4.png"><br>
	</div>

	<div id="badBrowser" style="display:none">
		<span style='color:red; font-size:larger;'>Incompatible browser</span><br>
		please, download <A href='https://s3.amazonaws.com/cdn.thewarinc.com/Install/WarInc_WebSetup.exe'>installer</A> manually<br>
		or launch the game by shortcut
	</div>
	<div id="badOS" style="display:none">
		<span style='color:red; font-size:larger;'>Incompatible operating system</span><br>
		The game runs only using Windows XP or higher
	</div>
</div>


<div id="applet"></div>

<script type="text/javascript"> 
	//document.getElementById("browser").innerHTML = "Browser: " + navigator.userAgent;

function testForActiveX()
{
       	tester = null;
        try {
		tester = new ActiveXObject('WarIncLauncher.WarIncLauncher.1');  
	}
       	catch (e) {}
	if (tester) {
		delete tester;
		tester = null;
		return true;
	}
	return false;
}

function testForNetscapePlugin()
{
	for(var i=0; i<navigator.plugins.length; i++) {
		if(navigator.plugins[i].name=="WI game updater") {
			return true;
		}
	}
	return false;
}

if(navigator.userAgent.indexOf ("Windows") == -1)
{
	document.getElementById("badOS").style.display = "block";
}
else if(navigator.userAgent.indexOf("MSIE")!=-1)
{
       	document.getElementById("applet").innerHTML =
		"<OBJECT width=0 height=0 id='WarIncLauncher' \
		CLASSID='CLSID:64F2ECD6-505E-4A9A-9606-4727B70DF3E9' \
		CODEBASE='http://cdnthewarinc.net/playnow/WarIncLaunch_1_0_0_9.cab#Version=1,0,0,9'> \
		<PARAM NAME='cmdLine' VALUE='<?echo $cmd?>'></OBJECT>";

	//if(testForActiveX() == false)
	//	document.getElementById("ieApplet").style.display = "block";
	//else
	//	document.getElementById("welcome").style.display = "block";

}
else if(navigator.userAgent.indexOf("Firefox")!=-1)
{
	if(testForNetscapePlugin() == false)
		document.getElementById("ffApplet").style.display = "block";
	else
		document.getElementById("welcome").style.display = "block";

	document.getElementById("applet").innerHTML =
		"<embed type='application/npwiu' pluginspage='https://account.thewarinc.com/playnow/npwiu_1_0_0_3.xpi' width=0 height=0 cmdLine='<?echo $cmd;?>'>";
}
else
{
	document.getElementById("badBrowser").style.display = "block";
}
</script>

</BODY>
</HTML>
