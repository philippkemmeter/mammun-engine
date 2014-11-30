<?


require_once(dirname(__FILE__).'/SimpleImage.inc');

class Image extends SimpleImage {
	var $image;
	var $ext;
	var $error = 0;


	function __construct($fname) {
		parent::__construct($fname);

		if (strpos($this->filename,".jpg") !== false or strpos($this->filename,".jpeg") !== false) {
			$this->image = @ImageCreateFromJpeg($this->filename);
			$this->ext = ".jpg";
		}
		elseif (strpos($this->filename,".png") !== false) {
			$this->image = @ImageCreateFromPng($this->filename); // $img
			/*      $sz = getimagesize($this->filename);
			$bgsz = ($sz[0] > $sz[1]) ? $sz[0] : $sz[1];
			$this->image = ImageCreateTrueColor($bgsz, $bgsz);

			$bgcolor = imagecolorallocate($this->image, 0x20, 0x20, 0x50);
			imagefill($this->image, 0, 0, $bgcolor);

			imagecopymerge($this->image, $img, 0, 0, 0, 0, $sz[0], $sz[1], 100);
			*/
			$this->ext = ".png";
		}
		elseif (strpos($this->filename,".gif") !== false) {
			$this->image = @ImageCreateFromGif($this->filename);
			/*      $sz = getimagesize($this->filename);
			 $bgsz = ($sz[0] > $sz[1]) ? $sz[0] : $sz[1];
			 $this->image = ImageCreateTrueColor($bgsz, $bgsz);

			 $bgcolor = imagecolorallocate($this->image, 0x20, 0x20, 0x50);
			 imagefill($this->image, 0, 0, $bgcolor);

			 imagecopymerge($this->image, $img, 0, 0, 0, 0, $sz[0], $sz[1], 100); */
			$this->ext = ".gif";
		}
		else {

			$this->error = IERR_WRONG_EXT;
			return  false;
		}

	}


	/**
	 * Verschiebt das verkleinerte/-größerte Bild. Die Proportionen
	 * bleiben erhalten.
	 *
	 * $max_width:	maximale Breite
	 * $max_height:	   "     Höhe
	 * $target:		wohin zu verschieben. Ext wird automatisch angehängt.
	 *			Ist $target.$this->ext == $this->filename,
	 * 			findet keine Verschiebung statt.
	 */
	 
	function resize_and_mv($max_width=100, $max_height=100, $target='') {
		if (empty($target)) {
			$target = UPLD_PIC_DIR.User::get_current_user()->get_nick().'/pic_';
		}
		if (!file_exists($target))
			mkdir($target);
		$new_width = round(($this->width * $max_height) / $this->height);

		if ($new_width > $max_width) {
			$new_width = $max_width;
			$new_height = round(($this->height * $max_width) / $this->width);
		}
		else
		$new_height = $max_height;

		$thumb = @imagecreatetruecolor ($new_width, $new_height);

		if (!@ImageCopyResampled($thumb, $this->image, 0, 0, 0, 0, $new_width, $new_height, $this->width, $this->height))
		@ImageCopyResized($thumb, $this->image, 0, 0, 0, 0, $new_width, $new_height, $this->width, $this->height);

		$fname = $target.$this->ext;
		if ($fname != $this->filename) {	// Bei Verschiebung, neuen Dateinamen
			$i = 0;							// ermitteln, dass nix überschieben
			while (file_exists($fname)) {	// wird
				$i++;
				$fname = $target.$i.$this->ext;
			}
		}
		$target = $fname;

		switch ($this->ext) {
			case ".jpg": {
				imagejpeg($thumb, $target, 70);
				break;
			}
			case ".gif": {
				imagegif($thumb, $target);
				break;
			}
			case ".png": {
				imagepng($thumb, $target);
				break;
			}
			default: {
				$this->error = IERR_UNKNOWN;
				return  false;		// interner Fehler
			}
		}

		if ($fname != $this->filename) // Bei verschiebung alte Datei löschen
		@unlink($this->filename);
		$this->width = $new_width;
		$this->height = $new_height;
		$this->filename = $target;
		$this->image = $thumb;
		$this->filesize = filesize($target);
		return true;
	}
}

?>