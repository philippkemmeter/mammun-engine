<?
include ('../include/shared/classes/PV_encryption.inc');
include ('../include/classes/ABN.inc');
$cryptObj=new PV_encryption();
$expected_data = array(
		       'zahl'         => 'eint'
		       );
$GP=$cryptObj->load_GET_POST($expected_data);
/**
 * @deprecated 
 * header('Content-Type: image/jpeg');
imagejpeg(ABN::get_abn_img($GP['zahl']));
*/
header('Content-Type: image/png');
$img = ABN::get_abn_img($GP['zahl']);
imagepng($img);
imagedestroy($img);
?>
