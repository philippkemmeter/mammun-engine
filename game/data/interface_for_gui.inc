<?php
require_once ('../include/classes/InterfaceForGui.inc');
require_once ('../include/shared/classes/PV_encryption.inc');
require_once ('../include/shared/classes/server_com/ClientServer.inc');
require_once ('../include/shared/classes/datastructures/WorldCoordinates.inc');
$cryptObj = new PV_encryption();
$expected_data_with_session = array (
	'set_language'							=>	'(e)serialized',
	'get_user_data'							=>	'(e)serialized',
	'get_user_personal'						=>	'(e)serialized',
	'set_user_personal'						=>	'(e)serialized',
	'get_user_auth_keys'					=>	'(e)serialized',
	'add_user_auth_key'						=>	'(e)serialized',
	'set_email_address'						=>	'(e)serialized',
	'get_template_data'						=>	'(e)serialized',
	'get_template_data_t'					=>	'(e)serialized',
	'cmd_tutorial_text_read'				=>	'(e)serialized',
	'get_map_data'							=>	'(e)serialized',
	'get_dominated_sectors'					=>	'(e)serialized',
	'get_resources'							=>	'(e)serialized',
	'get_resource_gains'					=>	'(e)serialized',
	'get_map_ranking'						=>	'(e)serialized',
	'get_map_score'							=>	'(e)serialized',
	'get_league_score'						=>	'(e)serialized',
	'get_map_score_gains'					=>	'(e)serialized',
	'get_map_players_data'					=>	'(e)serialized',
	'cmd_tribe_morph'						=>	'(e)serialized',
	'cmd_tribe_change_clothes'				=>	'(e)serialized',
	'get_tribe_clothes'						=>	'(e)serialized',
	'cmd_tribes_insert_new_tribe'			=>	'(e)serialized',
	'get_tribes_in_mf'						=>	'(e)serialized',
	'get_tribe_in_mf_data'					=>	'(e)serialized',
	'get_tribes_data'						=>	'(e)serialized',
	'get_science_project_data'				=>	'(e)serialized',
	'get_science_data'						=>	'(e)serialized',
	'cmd_science_start_research'			=>	'(e)serialized',
	'cmd_science_stop_research'				=>	'(e)serialized',
	'get_techtree_data'						=>	'(e)serialized',
	'get_researchable_scs_row'				=>	'(e)serialized',
	'get_allowed_abilities_of_race'			=>	'(e)serialized',
	'get_sector_data'						=>	'(e)serialized',
	'get_buildable_buildings_data'			=>	'(e)serialized',
	'get_all_non_custom_buildings_data'		=>	'(e)serialized',
	'get_buildings_type_to_constkey_map'	=>	'(e)serialized',
	'get_buildings_in_sector_data'			=>	'(e)serialized',
	'cmd_buildings_construct'				=>	'(e)serialized',
	'cmd_buildings_change_color'			=>	'(e)serialized',
	'cmd_buildings_destroy'					=>	'(e)serialized',
	'get_moving_units'						=>	'(e)serialized',
	'cmd_move_tribe'						=>	'(e)serialized',
	'cmd_stop_tribe'						=>	'(e)serialized',
	'cmd_change_player_color'				=>	'(e)serialized',
	'get_map_chooser_ffa_data'				=>	'(e)serialized',
	'cmd_map_chooser_ffa_choose_map'		=>	'(e)serialized',
	'cmd_map_chooser_tag_team_choose_random_map'
											=>	'(e)serialized',
	'cmd_map_chooser_ffa_unchoose_map'		=>	'(e)serialized',
	'cmd_map_chooser_ffa_toggle_ready_state'=>	'(e)serialized',
	'get_map_chooser_tag_team_data'			=>	'(e)serialized',
	'cmd_map_chooser_tag_team_choose_map'	=>	'(e)serialized',
	'cmd_map_chooser_tag_team_choose_map_by_auth_key'
											=>	'(e)serialized',
	'get_map_chooser_tag_team_data_by_auth_key'
											=>	'(e)serialized',
	'get_map_chooser_tag_team_auth_key'		=>	'(e)serialized',
	'cmd_map_chooser_tag_team_unchoose_map'	=>	'(e)serialized',
	'get_map_chooser_private_data'			=>	'(e)serialized',
	'cmd_map_chooser_private_choose_random_map'
											=>	'(e)serialized',
	'cmd_map_chooser_private_choose_map_by_auth_key'
											=>	'(e)serialized',
	'get_map_chooser_private_data_by_auth_keys'
											=>	'(e)serialized',
	'get_map_chooser_private_auth_key'		=>	'(e)serialized',
	'cmd_map_chooser_private_unchoose_map'	=>	'(e)serialized',
	'cmd_map_chooser_private_open_new_map'	=>	'(e)serialized',
	'get_reserved_sectors'					=>	'(e)serialized',
	'cmd_logout'							=>	'(e)serialized',
	'cmd_set_bug_state'						=>	'(e)serialized',
	'cmd_set_bug_priority'					=>	'(e)serialized',
	'cmd_set_bug_cat_id'					=>	'(e)serialized',
	'cmd_add_bug_attachment'				=>	'(e)serialized',
	'cmd_get_bug_attachment'				=>	'(e)serialized',
	'cmd_join_bugs'							=>	'(e)serialized',
	'get_notes_data'						=>	'(e)serialized',
	'cmd_set_notes_data'					=>	'(e)serialized',
	'cmd_leave_map'							=>	'(e)serialized',
	'cmd_select_all_sciences_auto'			=>	'(e)serialized',
	'cmd_finish_science_choose'				=>	'(e)serialized',
	'cmd_accounting_delete'					=>	'(e)serialized',
	'cmd_invite_friends'					=>	'(e)serialized',
	'get_invited_friends'					=>	'(e)serialized',
	'get_event_log'							=>	'(e)serialized',
	'get_event_messages'					=>	'(e)serialized',
	'add_event_message'						=>	'(e)serialized',
	'delete_event_message'					=>	'(e)serialized',
	'mark_event_message_as_read'			=>	'(e)serialized',
	'get_maps_won_by_user'					=>	'(e)serialized',
	'get_most_expensive_units'				=>	'(e)serialized',
	'get_current_maps_of_users'				=>	'(e)serialized',
	'get_item_restrictions'					=>	'(e)serialized',
	'get_item_statistics'					=>	'(e)serialized',
	'get_medals'							=>	'(e)serialized',
	'get_user_medals'						=>	'(e)serialized',
	'get_level_up_present'					=>	'(e)serialized',
	'tutorial_magic1'						=>	'(e)serialized',
	'cmd_request_invitation'				=>	'(e)serialized'
);
$expected_data_without_session = array (
	'get_nick_by_fb_id'						=>	'(e)serialized',
	'cmd_register_user'						=>	'(e)serialized',
	'cmd_report_bug'						=>	'(e)serialized',
	'get_bug_reports'						=>	'(e)serialized',
	'get_bug_report'						=>	'(e)serialized',
	'cmd_activate_user'						=>	'(e)serialized',
	'cmd_set_new_pass'						=>	'(e)serialized',
	'cmd_connect_account_with_fb'			=>	'(e)serialized',
	'get_worlds'							=>	'(e)serialized'
);
$GP1 = $cryptObj->load_GET_POST($expected_data_with_session);
$GP2 = $cryptObj->load_GET_POST($expected_data_without_session);
if ($GP1) {
	require_once ('../include/accesscontrol2.inc');
}
$GP = array_merge($GP1, $GP2);

$r = new ClientServerResponse();
if (empty($GP)) {
	echo "<b>Warning</b>: No known command transmitted.<br/>\n";
	$data = (!empty($_GET)) ? $_GET : $_POST;
	echo "<br/>\nThe following transmitted commands are unknown:<br/>\n";
	foreach ($data as $command => $params) {
		if ($command === 'PHPSESSID')
			continue;
		$params = unserialize($params);
		if (is_array($params))
			echo "- $command(".implode(', ', $params).")<br/>\n";
		else
			echo "- $command($params)<br/>\n";
	}
	echo "<br/>\n";
}
else {
	foreach ($GP as $command => $params) {
		try {
			//Attachment wird komplett RAW übermittelt ohne Response-Header.
			if ($command == "cmd_get_bug_attachment")
				die(InterfaceForGui::$command($params));

			// Alles andere als ClientServerResponse übermitteln
			$r->add_value(InterfaceForGui::$command($params));
		}
		catch (Exception $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception($e);
		}
		catch (ExceptionWithErrorlog $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception(new Exception($e->getMessage(), $e->getCode()));
		}
		catch (ExceptionWithErrormail $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception(new Exception($e->getMessage(), $e->getCode()));
		}
		catch (HackattackException $e) {
			error_log("File: {$e->getFile()}, Line: {$e->getLine()}, ".
				"Message: {$e->getMessage()}, Code: {$e->getCode()}");
			$r->add_exception($e);
		}
	}
}
echo $r;
?>