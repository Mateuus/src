<?php
	$server_key = $_POST['serverkey'];
	if ($server_key != "CfFkqQWjfgksYG56893GDhjfjZ20") 
	{
		echo "WRONG SERVER KEY";
		exit();
	}

	header("Content-type: text/xml"); 

	$xml_output = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"; 
	$xml_output .= "<DB>\n"; 

	require_once('dbinfo.inc.php');

	// create & execute query: WEAPONS
	$tsql   = "select * from Items_Weapons";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<WeaponsArmory>\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');

		$muzzlepart=$member['MuzzleParticle'];

		$anim=$member['Animation'];
		$sound1=$member['Sound_Shot'];
		$sound2=$member['Sound_Reload'];

		$BulletID=$member['BulletID'];
		$Damage=$member['Damage'];
		$isImmediate="true";
		if ($member['isImmediate'] == 0) $isImmediate="false";
		$Mass=$member['Mass'];
		$Speed=$member['Speed'];
		$DamageDecay=$member['DamageDecay'];
		$Area=$member['Area'];
		$Delay=$member['Delay'];
		$Timeout=$member['Timeout'];
		$NumClips=$member['NumClips'];
		$Clipsize=$member['Clipsize'];
		$ReloadTime=$member['ReloadTime'];
		$ActiveReloadTick=$member['ActiveReloadTick'];
		$RateOfFire=$member['RateOfFire'];
		$Spread=$member['Spread'];
		$Recoil=$member['Recoil'];

		$GR1=$member['NumGrenades'];
		$GR2=$member['GrenadeName'];

		$Scope=$member['ScopeType'];
		$ScopeZoom=$member['ScopeZoom'];

		$FM=$member['Firemode'];
			
		$LevelRequired=$member['LevelRequired'];

		list($xx, $yy, $zz) = sscanf($member['MuzzleOffset'], "%f %f %f");

		$xml_output .= "\t<Weapon itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
		$xml_output .= "\t<Model file=\"Data/ObjectsDepot/Weapons/" . $fname . ".sco\" muzzlerOffset.x=\"".$xx."\" muzzlerOffset.y=\"".$yy."\" muzzlerOffset.z=\"".$zz."\"/>\n";
		$xml_output .= "\t<MuzzleModel file=\"". $muzzlepart . "\" />\n";
    		$xml_output .= "\t<HudIcon file=\"\$Data/Weapons/HudIcons/" . $fname . ".dds\"/> \n";
    		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\"/>\n";
   		$xml_output .= "\t<PrimaryFire bullet=\"".$BulletID."\" damage=\"".$Damage."\" immediate=\"".$isImmediate."\" mass=\"".$Mass."\" decay=\"".$DamageDecay."\" speed=\"".$Speed."\" area=\"".$Area."\" delay=\"".$Delay."\" timeout=\"".$Timeout."\" numShells=\"".$NumClips."\" clipSize=\"".$Clipsize."\" reloadTime=\"".$ReloadTime."\" activeReloadTick=\"".$ActiveReloadTick."\" rateOfFire=\"".$RateOfFire."\" spread=\"".$Spread."\" recoil=\"".$Recoil."\" numgrenades=\"".$GR1."\" grenadename=\"".$GR2."\" firemode=\"".$FM."\"  ScopeType=\"".$Scope."\"  ScopeZoom=\"".$ScopeZoom."\" />\n";
		$xml_output .= "\t<Animation type=\"" .$anim."\"/>\n";
		$xml_output .= "\t<Sound shoot=\"" .$sound1."\" reload=\"".$sound2."\"/>\n";

	  	$xml_output .= "\t</Weapon>\n"; 
	}               
	$xml_output .= "</WeaponsArmory>\n\n"; 

	// create & execute query: GEARS
	$tsql   = "select * from Items_Gear";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<GearArmory>\n\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');

		$DmgPerc=$member['DamagePerc'];
		$DmgMax=$member['DamageMax'];
		$Weight=$member['Weight'];
		$Bulkiness=$member['Bulkiness'];
		$Inaccuracy=$member['Inaccuracy'];
		$Stealth=$member['Stealth'];
		$LevelRequired=$member['LevelRequired'];
		$ProtectionLevel=$member['ProtectionLevel'];
			
	    	$xml_output .= "\t<Gear itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
	    	$xml_output .= "\t<Model file=\"Data/ObjectsDepot/Characters/" . $fname . ".sco\" />\n";
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
		$xml_output .= "\t<Armor weight=\"" . $Weight . "\" damagePerc=\"" . $DmgPerc . "\" damageMax=\"" . $DmgMax . "\" bulkiness=\"" . $Bulkiness . "\" inaccuracy=\"" . $Inaccuracy . "\" stealth=\"" . $Stealth . "\" ProtectionLevel=\"".$ProtectionLevel."\" />\n";
	  	$xml_output .= "\t</Gear>\n"; 
	}               
	$xml_output .= "</GearArmory>\n\n"; 

	// create & execute query: ITEMS
	$tsql   = "select * from Items_Generic";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<ItemsDB>\n\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');
		$LevelRequired=$member['LevelRequired'];

	    	$xml_output .= "\t<Item itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
	  	$xml_output .= "\t</Item>\n"; 
	}               
	$xml_output .= "</ItemsDB>\n\n"; 

	// create & execute query: PACKAGES
	$tsql   = "select * from Items_Packages";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<PackagesDB>\n\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');
		$LevelRequired=$member['LevelRequired'];

	    	$xml_output .= "\t<Item itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
	  	$xml_output .= "\t</Item>\n"; 
	}               
	$xml_output .= "</PackagesDB>\n\n";

	$xml_output .= "</DB>\n\n"; 

	echo $xml_output; 
	exit();
?>