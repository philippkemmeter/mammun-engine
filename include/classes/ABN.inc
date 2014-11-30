<?php

/**
 * ABN (Captcha)
 *
 */
class ABN {
	/**
	 * Generiert den Captcha
	 *
	 * @param uint $number
	 * @return binary (Image)
	 */
	static public function get_abn_img($number) {
		/**
		 * @deprecated 
		$img = imagecreate(40,16);
		$bgcolor = imagecolorallocate($img, 32,32,80);
		$red = imagecolorallocate($img, 239,0,0);
		imagestring($img, 6, 1, 1, $number, $red);
		*/
		
		$img = @imagecreate(40, 16);
		$background_color = imagecolorallocate($img, 32,32,80);
		$text_color = imagecolorallocate($img, 125, 174, 255);
		imagecolortransparent($img, $background_color);
		imagestring($img, 6, 1, 1,  $number, $text_color);
		return $img;
	}
}

?>