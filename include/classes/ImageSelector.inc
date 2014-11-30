<?


require_once(dirname(__FILE__).'/SimpleImage.inc');
/**
 * Diese Klasse stellt ein kleines Modul dar, dass die Bilder, die ein User
 * bereits hochgeladen hat, verwaltet.
 */

class ImageSelector {
	const DEBUG=false;
	
	// Upload-Fehlerkonstanten
	const UPLD_ERR_FILE_NOT_FOUND = 1;
	const UPLD_ERR_WRONG_EXT = 2;
	const UPLD_ERR_IMG_SIZE = 3;
	const UPLD_ERR_NO_FILE_ARRIVED = 4;
	const UPLD_ERR_FILE_NOT_UPPED_PER_VALID_FORM = 5;
	const UPLD_ERR_UNKNOWN = -1;
	
	// Lösch-Fehlerkonstanten
	const DEL_ERR_INVALID_FILE_NAME = 1;
	const DEL_ERR_UNLINK_FAILED = 2;
	
	const ACT_DELETE = 1;
	const ACT_UPLOAD = 2;
	const ACT_SELECT = 3;
	
	private $error = 0;		// Fehler, der aufgetreten ist
	private $action = 0;	// Aktion, die ausgeführt wurde
	private $images = array();
	private $reset_image;
	private $head_line;	// Die Überschrift der ImageSelector-Box
	private $get_params_upload=array();	// Zusätzliche Parameter, die beim Hochladen
								// mitgeschickt werden sollen
	private $get_params_sumbit=array();	// Zusätzliche Paramter, die bei der Bildwahl
								// mitgeschickt werden sollen
	private $get_params_delete=array();	// Zusätzliche Paramter, die beim Löschen
								// mitgeschickt werden sollen
		
	private $nick;
	
	static private $INSTANCE = array();	// instanzüberwachung. Nur eine Instanz pro UID innerhalb von PV
 										// Konstruktor ist private. Objekt statisch über get_instance($uid)
 										// anfordern! (s.u.)

 	/* Diese Statische Funtion als Konstruktor benutzen !*/
 	public static function get_instance($nick) {
 		if (self::DEBUG) {
			echo "Building: User id@get_instance: $nick<br>\n";
 		}
 		if (!self::$INSTANCE || !self::$INSTANCE[$nick]) {
 			self::$INSTANCE[$nick] = new ImageSelector($nick);
 		}
 		return self::$INSTANCE[$nick];
 	}

	private function __construct($nick) {
		$this->nick = $nick;
		$this->reset_image = false;
	}

	/**
	 * Wenn über den ImageSelector ein Bild hochgeladen wurde, dann wird
	 * das hier verwaltet.
	 * 
	 * @return mixed Gibt den Dateinamen zur neuhochgeladenen Datei zurück oder false im
	 * Fehlerfall. Dann ist der genaue Fehler über ImageSelector::get_error()
	 * abfragbar.
	 */
	private function handle_upload() {
		$this->action = self::ACT_UPLOAD;
		if (empty ($_FILES['file']['name'])) {
			$this->error = self::UPLD_ERR_NO_FILE_ARRIVED;
			return false;
		}
		$dest = CONF_UPLD_TMP_DIR.$_FILES["file"]["name"];
		@move_uploaded_file($_FILES["file"]["tmp_name"], $dest);
		
		$img = new Image($dest);
		if ((!$img->error) && ($img->resize_and_mv(100, 100, CONF_UPLD_PIC_DIR.$this->nick.'/pic_')))
			return $img->get_file_name();
		
		switch ($img->error) {
			case IERR_FILE_NOT_FOUND:
				$this->error = self::UPLD_ERR_FILE_NOT_FOUND;
				break;
			case IERR_IMG_SIZE:
				$this->error = self::UPLD_ERR_IMG_SIZE;
				break;
			case IERR_WRONG_EXT:
				$this->error = self::UPLD_ERR_WRONG_EXT;
				break;
			default:
				$this->error = self::UPLD_ERR_UNKNOWN;
		}
		return false;		
	}
	
	/**
	 * Löscht das gewählte Bild von der Festplatte.
	 * 
	 * @param $filename string Datei, die gelöscht werden soll
	 * @return mixed FALSE wenn's nicht geklappt hat, sonst Dateinamen
	 */
	private function delete_pic($filename) {
		$filename = CONF_UPLD_PIC_DIR.$this->nick.'/'.$filename;
		$this->action = self::ACT_DELETE;
		if (!@unlink($filename)) {
			$this->error = self::DEL_ERR_UNLINK_FAILED;
			echo $filename;
			return false;
		}
		return $filename;
	}
	
	/**
	 * Markiert die gewählte Datei als das ausgesuchte Bild und gibt
	 * es als solches zurück
	 *
	 * @param string $filename
	 * @return mixed FALSE, wenn nichte gewählt wurde, sont den Dateinamen
	 */
	private function submit_pic($filename) {
		$this->action = self::ACT_SELECT;
		if (!$filename)
			return false;
		$filename = CONF_UPLD_PIC_DIR.$this->nick.'/'.$filename;
		if (!file_exists($filename))
			return false;
		return $filename;
	}
	
	/**
	 * Enter description here...
	 *
	 * @return mixed
	 */
	public function check_for_actions() {
		if (isset($_GET['image_selector_upload']) || isset($_POST['image_selector_upload'])) {
			return $this->handle_upload();
		}
		elseif (isset($_GET['image_selector_delete'])) {
			return $this->delete_pic($_GET['image_selector_delete']);
		}
		elseif (isset($_POST['image_selector_delete'])) {
			return $this->delete_pic($_POST['image_selector_delete']);
		}
		elseif (isset($_GET['image_selector_submit'])) {
			return $this->submit_pic($_GET['image_selector_submit']);
		}
		elseif (isset($_POST['image_selector_submit'])) {
			return $this->submit_pic($_POST['image_selector_submit']);
		}
		else {
			$this->action = 0;
			return true;
		}
	}
	
	/**
	 * Adds an get parameter to the upload functionality
	 *  
	 * @param string $param the parameter (like "x=u" or "blah=0")
	 */
	public function add_get_param_on_upload($param) {
		$p = explode('=', $param);
		$this->get_params_upload[$p[0]] = $p[1];
	}
	
	/**
	 * Adds an get parameter to the image submit form
	 *  
	 * @param string $param the parameter (like "x=u" or "blah=0")
	 */
	public function add_get_param_on_submit($param) {
		$p = explode('=', $param);
		$this->get_params_sumbit[$p[0]] = $p[1];
	}
	
	/**
	 * Adds an get parameter to the image deletion form
	 *  
	 * @param string $param the parameter (like "x=u" or "blah=0")
	 */
	public function	add_get_param_on_delete($param) {
		$p = explode('=', $param);
		$this->get_params_delete[$p[0]] = $p[1];
	}
	
	public function draw() {
		$path = CONF_UPLD_PIC_DIR.$this->nick.'/';
		if (!file_exists($path))
			@mkdir($path);
		$img_filenames = @scandir($path);
		if ($img_filenames) {
			foreach ($img_filenames as $fname) {
				if ($fname{0} === '.')	// . und .. ignorieren und alle versteckten
					continue;
				$this->images[] = new SimpleImage($path.'/'.$fname);
			}
		}
		include ('templates/ImageSelector.tmpl.inc');
	}

	/*** GETTER ***/
	public function get_error() {
		return $this->error;
	}
	
	public function get_image_list() {
		return $this->images;
	}
	
	public function get_action() {
		return $this->action;
	}
	
	
	/*** SETTER ***/
	public function set_head_line($head_line) {
		$this->head_line = $head_line;
	}
	
	/**
	 * Das Reset-Pic ist ein Bild am Ende der Liste der Bilder. Bei Wahl dieses
	 * Bildes wird ein leerer Pfad zurückgegeben, statt des Pfades zum Bild.
	 * Die Idee ist, dass man ein Bild wählt, das die Wahl "Kein Bild" sugerriert. 
	 *
	 * @param SimpleImage $img
	 */
	public function set_reset_image($img) {
		$this->reset_image = $img;
	}
}


?>