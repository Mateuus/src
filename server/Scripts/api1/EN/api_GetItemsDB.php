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

	function xml_addvar($name, $db_name)
	{
		global $member;
		global $xml_output;

		$xml_output .= " $name=\"" . $member[$db_name] . "\"";
	}

	$xml_output .= "<WeaponsArmory>\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');
		$desc=str_replace("\r\n", "&#xD;", trim($desc));

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
		$IsUpgradeable=$member['IsUpgradeable'];

		list($xx, $yy, $zz) = sscanf($member['MuzzleOffset'], "%f %f %f");

		$AnimPrefix = $member['AnimPrefix'];

		$xml_output .= "\t<Weapon itemID=\"" . $itemid . "\" category=\"" . $cat . "\" upgrade=\"" . $IsUpgradeable . "\" FNAME=\"" . $fname . "\" >\n "; 
		$xml_output .= "\t<Model file=\"Data/ObjectsDepot/Weapons/" . $fname . ".sco\" AnimPrefix=\"" . $AnimPrefix . "\" muzzlerOffset.x=\"".$xx."\" muzzlerOffset.y=\"".$yy."\" muzzlerOffset.z=\"".$zz."\"/>\n";
		$xml_output .= "\t<MuzzleModel file=\"". $muzzlepart . "\" />\n";
    		$xml_output .= "\t<HudIcon file=\"\$Data/Weapons/HudIcons/" . $fname . ".dds\"/> \n";
    		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\"/>\n";
   		$xml_output .= "\t<PrimaryFire bullet=\"".$BulletID."\" damage=\"".$Damage."\" immediate=\"".$isImmediate."\" mass=\"".$Mass."\" decay=\"".$DamageDecay."\" speed=\"".$Speed."\" area=\"".$Area."\" delay=\"".$Delay."\" timeout=\"".$Timeout."\" numShells=\"".$NumClips."\" clipSize=\"".$Clipsize."\" reloadTime=\"".$ReloadTime."\" activeReloadTick=\"".$ActiveReloadTick."\" rateOfFire=\"".$RateOfFire."\" spread=\"".$Spread."\" recoil=\"".$Recoil."\" numgrenades=\"".$GR1."\" grenadename=\"".$GR2."\" firemode=\"".$FM."\"  ScopeType=\"".$Scope."\"  ScopeZoom=\"".$ScopeZoom."\" />\n";
		$xml_output .= "\t<Animation type=\"" .$anim."\"/>\n";
		$xml_output .= "\t<Sound shoot=\"" .$sound1."\" reload=\"".$sound2."\"/>\n";
		$xml_output .= "\t<FPS";
			$IsFPS = $member['IsFPS'];
			if($IsFPS > 0)
			{
				xml_addvar('IsFPS', 'IsFPS');
				xml_addvar('i0', 'FPSSpec0');
				xml_addvar('i1', 'FPSSpec1');
				xml_addvar('i2', 'FPSSpec2');
				xml_addvar('i3', 'FPSSpec3');
				xml_addvar('i4', 'FPSSpec4');
				xml_addvar('i5', 'FPSSpec5');
				xml_addvar('i6', 'FPSSpec6');
				xml_addvar('i7', 'FPSSpec7');
				xml_addvar('i8', 'FPSSpec8');

				xml_addvar('d0', 'FPSAttach0');
				xml_addvar('d1', 'FPSAttach1');
				xml_addvar('d2', 'FPSAttach2');
				xml_addvar('d3', 'FPSAttach3');
				xml_addvar('d4', 'FPSAttach4');
				xml_addvar('d5', 'FPSAttach5');
				xml_addvar('d6', 'FPSAttach6');
				xml_addvar('d7', 'FPSAttach7');
				xml_addvar('d8', 'FPSAttach8');
			}
		$xml_output .= "/>\n";

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
		$desc=str_replace("\r\n", "&#xD;", trim($desc));

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
		$desc=str_replace("\r\n", "&#xD;", trim($desc));

		$LevelRequired=$member['LevelRequired'];

		$xml_output .= "\t<Item itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
		if($cat == 4 || $cat==7) // storecat_Items, storecat_LootBox
		{
			$xml_output .= "\t<Model file=\"Data/ObjectsDepot/Weapons/" . $fname . ".sco\" />\n";
		}	
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
	  	$xml_output .= "\t</Item>\n"; 
	}               
	$xml_output .= "</ItemsDB>\n\n"; 

	// create & execute query: Attachments
	$tsql   = "select * from Items_Attachments";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<AttachmentArmory>\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$type=$member['Type'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');
		$desc=str_replace("\r\n", "&#xD;", trim($desc));

		$MuzzleParticle=$member['MuzzleParticle'];
		$FireSound=$member['FireSound'];
		$Damage=$member['Damage'];
		$Range=$member['Range'];
		$Firerate=$member['Firerate'];
		$Recoil=$member['Recoil'];
		$Spread=$member['Spread'];
		$Clipsize=$member['Clipsize'];
		$ScopeMag=$member['ScopeMag'];
		$ScopeType=$member['ScopeType'];
		$ScopeAnimPrefix=$member['AnimPrefix'];
		$SpecID=$member['SpecID'];
		
		$LevelRequired=$member['LevelRequired'];
			
		$xml_output .= "\t<Attachment itemID=\"" . $itemid . "\" type=\"" . $type . "\" SpecID=\"" . $SpecID . "\" >\n "; 
		$xml_output .= "\t<Model file=\"Data/ObjectsDepot/Weapons/" . $fname . ".sco\" MuzzleParticle=\"" .$MuzzleParticle. "\" FireSound=\"".$FireSound."\" ScopeAnim=\"".$ScopeAnimPrefix."\" />\n";
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
		$xml_output .= "\t<Upgrade damage=\"" . $Damage . "\" range=\"" . $Range . "\" firerate=\"" . $Firerate . "\" recoil=\"" . $Recoil . "\" spread=\"" . $Spread . "\" clipsize=\"" . $Clipsize . "\" ScopeMag=\"".$ScopeMag."\" ScopeType=\"".$ScopeType."\" />\n";
	  	$xml_output .= "\t</Attachment>\n"; 
	}               
	$xml_output .= "</AttachmentArmory>\n\n"; 
		
	// create & execute query: PACKAGES
	$tsql   = "select * from Items_Packages";
	$params = array();
	$stmt   = sqlsrv_query($conn, $tsql, $params);

	$xml_output .= "<PackagesDB>\n"; 
	while($member = sqlsrv_fetch_array($stmt, SQLSRV_FETCH_ASSOC))
	{
		$itemid=$member['ItemID'];
		$fname=str_replace (" ", "",$member['FNAME']);
		$cat=$member['Category'];
		$name=htmlentities($member['Name'], ENT_COMPAT, 'UTF-8');
		$desc=htmlentities($member['Description'], ENT_COMPAT, 'UTF-8');
		$desc=str_replace("\r\n", "&#xD;", trim($desc));

		$LevelRequired=$member['LevelRequired'];
		$addGP=$member['AddGP'];
		$addSP=$member['AddSP'];
		$item1=$member['Item1_ID'];
		$item1Exp=$member['Item1_Exp'];
		$item2=$member['Item2_ID'];
		$item2Exp=$member['Item2_Exp'];
		$item3=$member['Item3_ID'];
		$item3Exp=$member['Item3_Exp'];
		$item4=$member['Item4_ID'];
		$item4Exp=$member['Item4_Exp'];
		$item5=$member['Item5_ID'];
		$item5Exp=$member['Item5_Exp'];
		$item6=$member['Item6_ID'];
		$item6Exp=$member['Item6_Exp'];
		
		$xml_output .= "\t<Package itemID=\"" . $itemid . "\" category=\"" . $cat . "\"  >\n "; 
		$xml_output .= "\t<Store name=\"" . $name . "\" icon=\"\$Data/Weapons/StoreIcons/" . $fname . ".dds\" desc=\"".$desc."\" LevelRequired=\"".$LevelRequired."\" />\n";
		$xml_output .= "\t<PackageDesc gp=\"" . $addGP . "\" sp=\"".$addSP."\" item1ID=\"".$item1."\" item1Exp=\"".$item1Exp."\" item2ID=\"".$item2."\" item2Exp=\"".$item2Exp."\" item3ID=\"".$item3."\" item3Exp=\"".$item3Exp."\" item4ID=\"".$item4."\" item4Exp=\"".$item4Exp."\" item5ID=\"".$item5."\" item5Exp=\"".$item5Exp."\" item6ID=\"".$item6."\" item6Exp=\"".$item6Exp."\" />\n";
	  	$xml_output .= "\t</Package>\n"; 
	}               
	$xml_output .= "</PackagesDB>\n\n";

	$xml_output .= "</DB>\n\n"; 

	echo $xml_output; 
	exit();
?>