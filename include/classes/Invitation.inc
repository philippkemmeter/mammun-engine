<?php
require_once(dirname(__FILE__).'/../shared/classes/Language.inc');

Language::get_instance()->load_language_file('Invitation_');
/**
 * Diese Klasse ist dafür da, um Weitere Mitspieler einzuladen.
 *
 * @author Philipp Kemmeter
 */
class Invitation {
	private $uid;


	private static $INSTANCES;

	/**
	 * Gibt die Instanz für den User zurück, der einladen möchte
	 *
	 * @param uint $uid
	 * @return Invitation
	 */
	public static function get_instance($uid) {
		if (!isset(self::$INSTANCES[$uid]))
			self::$INSTANCES[$uid] = new self($uid);
		return self::$INSTANCES[$uid];
	}

	/**
	 * Erzeugt ein neues Invitation-Objekt. Diese Funktion ist privat, da
	 * Singleton.
	 *
	 * @param uint $uid
	 * @return Invitation
	 */
	private function __construct($uid) {
		$this->uid = $uid;
	}

	/**
	 * Sendet die Einladung an die Freunde ab. Ein Freund muss dabei ein Objekt
	 * folgender Struktur sein:
	 *
	 * $o->name		Name des Freundes (so wird er angesprochen, optional)
	 * $o->email	E-Mail-Adresse des Freundes
	 *
	 * ODER
	 *
	 * $o->mammun_nick	Freund ist in Mammun registriert. Es muss nix weiter
	 * 					angegeben werden, weil das dann aus der DB gelesen wird.
	 *
	 * Der zweite Parameter gibt einen Text an, der zusätzlich zum Standard-
	 * einladungstext erscheinen soll (das wird der User derart gewäht haben).
	 *
	 * Wird $direct_to_my_map angegeben, wird statt einer generellen Einladung
	 * zu Mammun, eine Einladung direkt auf die Karte von "mir" geschickt, um
	 * in meinem Team mitzuspielen - falls möglich.
	 *
	 * @param stdClass[] $friends
	 * @param string $additional_text
	 * @param bool $direct_to_my_map
	 */
	public function send_invitation_to_friends(array $friends,
		$additional_text, $direct_to_my_map=false)
	{
		$user = User::get_instance_by_id($this->uid);
		if ($direct_to_my_map) {
			$auth_key = MapChooserPrivate
				::get_auth_key_for_current_map($this->uid);
			/**
			 * Direkt-auf-Karte-Einladung
			 */
			foreach ($friends as $friend) {
				if (!is_object($friend))
					continue;

				if (!empty($friend->email)) {
					// Spam-Schutz
					try {
						query(
							"INSERT INTO invitation_map_email ".
								"SET uid={$this->uid}, ".
									"world_id={$user->get_world_id()}, ".
									"map_id={$user->get_map_id()}, ".
									"email='{$friend->email}'",
							true
						);
					}
					catch (ExceptionWithErrorlog $e) {
						/*throw new Exception(
							"User already invited", 1
						);*/
					}
					$friend_name = $friend->name ? " {$friend->name}" : '';
					$mail_text = str_replace(
						array(
							'[FRIEND_NAME]',
							'[INVITING_NAME]',
							'[TEXT]',
							'[MAP_KEY]',
							'<br/> ',
							'<br> ',
							'<br/>',
							'<br>'
						),
						array(
							$friend_name,
							$user->get_nick_main(),
							html_entity_decode($additional_text),
							$auth_key.$user->get_id(),
							"\n",
							"\n",
							"\n",
							"\n"
						),
						LNG_INVITATION_FRIENDS_MAIL_TO_MAP_CONTENT
					);
					$topic = str_replace(
						'[INVITING_NAME]',
						$user->get_nick(),
						LNG_INVITATION_FRIENDS_MAIL_TO_MAP_TOPIC
					);
				}
				else if (!empty($friend->mammun_nick)) {
					try {
						$friend_user = User::get_instance_by_nick(
							$friend->mammun_nick
						);
						$friend_user->add_map_invitation_auth_key($auth_key,
							$this->uid);
					}
					catch (Exception $e) {
						error_log("WARNING: Invitation.inc, "
							."Line: ".__LINE__.': '.$e->getMessage());
						continue;
					}
					try {
						// Spam-Schutz
						query(
							"INSERT INTO invitation_map_ingame ".
								"SET uid={$this->uid}, ".
									"world_id={$user->get_world_id()}, ".
									"map_id={$user->get_map_id()}, ".
									"uid2={$friend_user->get_id()}"
						);
					}
					catch (ExceptionWithErrorlog $e) {
						throw new Exception(
							"User already invited", 1
						);
					}
					/**
					 * Dem Spieler zusätzlich eine E-Mail schicken, obwohl er
					 * bei Mammun ist, damit er, falls inaktiv, davon Wind
					 * bekommt.
					 */
					$friend->email = $friend_user->get_email();
					$mail_text = str_replace(
						array(
							'[FRIEND_NAME]',
							'[INVITING_NAME]',
							'[EXP]',
							'[MAP_KEY]',
							'<br/> ',
							'<br> ',
							'<br/>',
							'<br>'
						),
						array(
							$friend_user->get_nick_main(),
							$user->get_nick_main(),
							$user->get_cur_level(),
							$auth_key,
							"\n",
							"\n",
							"\n",
							"\n"
						),
						LNG_INVITATION_MAMMUN_FRIEND_DIRECT_CONTENT
					);
					$topic = str_replace(
						'[INVITING_NAME]',
						$user->get_nick(),
						LNG_INVITATION_MAMMUN_FRIEND_DIRECT_TOPIC
					);

					/**
					 * Und jetzt noch als EventLog und als EventMessage ihm
					 * zuschicken. Dann kann er's nicht verpassen :)
					 *
					 * Er bekommt die Nachricht nur, wenn er auf einer Karte
					 * ist, da er sonst sowieso nichts anderes als Einladungen
					 * die ganze Zeit gezeigt bekommt.
					 */
					if ($friend_user->get_state() == User::STATE_PLAYING) {
						EventLog::get_instance($friend_user->get_id(),
							$friend_user->get_world_id(),
							$friend_user->get_map_id())->add_entry
						(
							EventLog::TYPE_MAP_INVITATION_RECEIVED,
							$user->get_map_id(),
							$user->get_nick().'-'.$user->get_world_id()
						);

						EventMessages::get_instance($friend_user->get_id())
							->add_entry
						(
							EventLog::TYPE_MAP_INVITATION_RECEIVED,
							$user->get_map_id(),
							$user->get_nick().'-'.$user->get_world_id()
						);
					}
				}
				else {
					continue;
				}
				pv_mail($friend->email, $topic, $mail_text, $user->get_email());
			}
		}
		else {
			/**
			 * Generelle Einladung
			 */
			foreach ($friends as $friend) {
				if ((!is_object($friend)) || (empty($friend->email)))
					continue;

				try {
					// Spam-Schutz
					query(
						"INSERT INTO invitation_mentor_email ".
							"SET uid={$this->uid}, ".
								"email='{$friend->email}'"
					);
				}
				catch (ExceptionWithErrorlog $e) {
					/*throw new Exception(
						"User already invited", 1
					);*/
				}

				$friend_name = $friend->name ? " {$friend->name}" : '';

				$mail_text = str_replace(
					array(
						'[FRIEND_NAME]',
						'[INVITING_NAME]',
						'[TEXT]',
						'<br/> ',
						'<br> ',
						'<br/>',
						'<br>',
						"\t"
					),
					array(
						$friend_name,
						$user->get_nick_main(),
						html_entity_decode($additional_text),
						"\n",
						"\n",
						"\n",
						"\n",
						''
					),
					LNG_INVITATION_FRIENDS_MAIL_CONTENT
				);
				pv_mail($friend->email, LNG_INVITATION_FRIENDS_MAIL_TOPIC,
					$mail_text, $user->get_email());
			}
		}
	}

	/**
	 * Löscht die Spam-Schutzdaten, dass der Spieler auf der angegebenen Karte
	 * schon eingeladene Spieler nochmal einladen darf.
	 *
	 * Das kann vorallem dann sinnvoll sein, wenn der Spieler die angegebene
	 * Karte verlassen hat, oder wenn er aus anderen Gründen keine Einladungen
	 * mehr verschicken kann. Somit kann die DB entschlackt werden.
	 *
	 * @param uint $map_id
	 */
	public function clear_spam_protect($world_id, $map_id) {
		$map_id = (int)$map_id;
		query(
			"DELETE FROM invitation_map_email WHERE uid={$this->uid} ".
				"AND world_id=$world_id AND map_id=$map_id"
		);
		query(
			"DELETE FROM invitation_map_fb WHERE uid={$this->uid} ".
				"AND world_id=$world_id AND map_id=$map_id"
		);
		query(
			"DELETE FROM invitation_map_ingame WHERE uid={$this->uid} ".
				"AND world_id=$world_id AND map_id=$map_id"
		);
	}

	/**
	 * Gibt die E-Mail-Adressen der per E-Mail eingeladenen Freunde zurück.
	 *
	 * Wird Map-ID angegeben, so werden die Einladungen auf die angegebene Karte
	 * zurückgegeben, sonst die generellen (Mentoren-) Einladungen.
	 *
	 * @return string[]
	 */
	public function get_invited_email($world_id=0, $map_id=0) {
		$world_id = (int)$world_id;
		$map_id = (int)$map_id;
		if ($world_id > 0) {
			$res = query(
				"SELECT email FROM invitation_map_email ".
					"WHERE uid={$this->uid} AND world_id=$world_id AND ".
						"map_id=$map_id"
			);
		}
		else {
			$res = query(
				"SELECT email FROM invitation_mentor_email ".
					"WHERE uid={$this->uid}"
			);
		}
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}

	/**
	 * Gibt die Facebook-IDs der per Facebook eingeladenen Freunde zurück.
	 *
	 * Wird Map-ID angegeben, so werden die Einladungen auf die angegebene Karte
	 * zurückgegeben, sonst die generellen (Mentoren-) Einladungen.
	 *
	 * @return uint[]
	 */
	public function get_invited_fb($world_id=0, $map_id=0) {
		$world_id = (int)$world_id;
		$map_id = (int)$map_id;
		if ($world_id > 0) {
			$res = query(
				"SELECT fb_id FROM invitation_map_fb ".
					"WHERE uid={$this->uid} AND world_id=$world_id ".
						"AND map_id=$map_id"
			);
		}
		else {
			$res = query(
				"SELECT fb_id FROM invitation_mentor_fb ".
					"WHERE uid={$this->uid}"
			);
		}
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}

	/**
	 * Gibt die User-IDs der ingame eingeladenen Freunde zurück.
	 *
	 * Hier muss immer eine Map-ID angegeben werden, weil man ingame keine Leute
	 * zu "Mammun" einladen kann (sie spielen ja schon), sondern nur auf eine
	 * Karte.
	 *
	 * @return uint[]
	 */
	public function get_invited_ingame($world_id, $map_id) {
		$map_id = (int)$map_id;
		$res = query(
			"SELECT uid2 FROM invitation_map_ingame ".
				"WHERE uid={$this->uid} AND world_id=$world_id ".
					"AND map_id=$map_id"
		);
		$result = array();
		while ($row = mysql_fetch_row($res)) {
			$result[] = $row[0];
		}
		return $result;
	}

}
?>