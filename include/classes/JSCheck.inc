<?

/**
 * Diese Klasse ist zum davon Erben gedacht. Sie verwaltet z.B. ob der script-Tag noch offen,
 * oder schon geschlossen ist, und gibt ihn entsprechend aus
 */
class JSCheck {
	private $js_tag_open = false;		/* Ob Script-Tag noch offen ist, oder nicht */
	private $keep_js_tag_open = false;	/* Ob der JS-Tag gezwungenermaßen offen bleiben soll */
	public function __construct() {
	}
	
  
	/**
	 * Gibt den öffnenden JS-Tag aus und setzt entsprechend die Variable
	 */
	public function open_js_tag() {
		if (!$this->js_tag_open) {
			echo '<script type="text/javascript">'."\n";
			$this->js_tag_open = true;
		}
	}
  
	/**
	 * Gibt den schließenden JS-Tag aus und setzt entsprechend die Variable
	 */
	public function close_js_tag() {
		if ($this->js_tag_open && !$this->keep_js_tag_open) {
			echo "</script>\n";
			$this->js_tag_open = false;
		}
	}
  
	/**
	 * Schaltet offenen und geschlossenen Tag um.
	 */
	public function toggle_js_tag() {
		if ($this->js_tag_open)
			close_js_tag();
		else
			open_js_tag();
	}

	public function set_keep_js_tag_open($setting) {
		$this->keep_js_tag_open = $setting;

		// Wenn offen gehalten werden soll, dann auch als offen Kennzeichnen
		if ($setting)
			$this->js_tag_open = true;
	}
	
	public function set_tag_style($style) {
	}
}

?>