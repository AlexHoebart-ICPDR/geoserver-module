<?php

/**
 * @file
 * This file holds the functions for the main GeoServer admin settings.
 *
 * @ingroup geoserver
 */

/**
 * Menu callback; displays the geoserver module settings page.
 *
 * @see system_settings_form()
 */
function geoserver_ui_admin_settings_form() {

  $url        = config_get('geoserver.settings', 'geoserver_url');
  $workspace  = config_get('geoserver.settings', 'geoserver_workspace');
  $workspaces = array('');
  $workspace_type = 'select';

  if (!empty($url)) {
    try {
      $result = geoserver_get('rest/workspaces.json');
      // Only use data if it is an array to take account for GeoServer sending an empty string instead of an empty array
      if (is_array($result->data->workspaces->workspace)) {
        foreach ($result->data->workspaces->workspace as $ws) {
          $workspaces[$ws->name] = $ws->name;
        }
      }
    }
    catch (geoserver_resource_exception $exc) {
      $workspace_type = 'textfield';
      backdrop_set_message(t('Failed to get list of workspaces from GeoServer. See the log for details.'), 'error');
      watchdog('geoserver', 'Failed to get list of workspaces from GeoServer: @exception',
        array('@exception' => $exc->getMessage()), WATCHDOG_ERROR);
    }
  }

  if (geoserver_check_jsonp() === FALSE) {
    backdrop_set_message(t('JSONP is disabled and some services might not work. See the GeoServer module documentation on <a href="@link">how to enable JSONP</a>.', array('@link' => 'https://www.drupal.org/node/1504464')), 'warning');
  }

  #$styles = geoserver_styles();
  #$layers = geoserver_layers_load();

  $form = array(
    'geoserver_url' => array(
      '#type' => 'textfield',
      '#title' => t('GeoServer URL'),
      '#default_value' => $url,
      '#description' => t('The URL where GeoServer is running. You need to re-login into Backdrop to allow authentication to GeoServer before the new server can be used.'),
    ),
    'geoserver_workspace' => array(
      '#type' => $workspace_type,
      '#title' => t('GeoServer Workspace'),
      '#description' => t('The workspace where all layers and styles will be stored. Only changeable as long as the list of layers and styles is empty.'),
      '#options' => $workspace_type == 'select' ? $workspaces : NULL,
      '#default_value' => $workspace,
      '#disabled' => !(empty($layers) && empty($styles)),
    ),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Submit handler for settings form.
 */
function geoserver_ui_admin_settings_form_submit(&$form, &$form_state) {
  $form_state['values']['geoserver_url'] = rtrim($form_state['values']['geoserver_url'], '/') . '/';
  foreach ($form_state['values'] as $name => $value) {
    config_set('geoserver.settings', $name, $value);
  }
}

