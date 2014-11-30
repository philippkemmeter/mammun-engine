<?php



//TODO Classconstants
define('IERR_FILE_NOT_FOUND', 1);
define('IERR_WRONG_EXT', 2);
define('IERR_IMG_SIZE', 3);
define('IERR_UNKNOWN', -1);
//TODO Exceptions

class SimpleImage {
	protected  $width;
	protected $height;
	protected $filename;
	protected $filesize;
	
	public function __construct($fname) {
		if (!file_exists($fname)) {
			$this->error = IERR_FILE_NOT_FOUND;
			return false;
		}
		$this->filename = $fname;
    
		if (($size = @getimagesize($this->filename)) == false) {
			$this->error = IERR_IMG_SIZE;
			return false;
		}
		$this->width = $size[0];
		$this->height = $size[1];
		$this->filesize = filesize($this->filename);
	}
	
	public function get_width() {
		return $this->width;
	}
	
	public function get_height() {
		return $this->height;
	}
	
	public function get_file_name() {
		return $this->filename;
	}
	
	public function get_size() {
		return $this->filesize;
	}
}


?>