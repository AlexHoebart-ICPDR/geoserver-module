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
function geoserver_ui_admin_settings() {

  $url        = variable_get('geoserver_url', '');
  $namespace  = variable_get('geoserver_namespace', '');
  $workspace  = variable_get('geoserver_workspace', '');
  $workspaces = array();
  
  if (!empty($url)) {
    $result = geoserver_get('rest/workspaces.json');
    if ($result->code == 200) {
      foreach ($result->data->workspaces->workspace as $ws) {
        $workspaces[$ws->name] = $ws->name;
      }
    }
    
    if (!empty($workspace)) {
      $result = geoserver_get('rest/namespaces/' . $workspace . '.json');
      if ($result->code == 200) {
        $namespace = $result->data->namespace->uri;
      }
    }
  }

  $form = array(
    'geoserver_url' => array(
      '#type' => 'textfield',
      '#title' => t('URL'),
      '#default_value' => $url,
      '#description' => 'The URL where GeoServer is running. Needs to end with a slash.',
    ),
    'geoserver_workspace' => array(
      '#type' => 'select',
      '#title' => t('Workspace'),
      '#options' => $workspaces,
      '#default_value' => $workspace,
      '#ajax' => array(
        'callback' => 'geoserver_ui_admin_settings_form_namespace',
        'wrapper' => 'geoserver_namespace'
      ),
    ),
    'geoserver_namespace' => array(
      '#type' => 'textfield',
      '#title' => t('Namespace URI'),
      '#disabled' => TRUE,
      '#default_value' => $namespace,
      '#prefix' => '<div id="geoserver_namespace">',
      '#suffix' => '</div>',
    ),
  );
  return system_settings_form($form);
}

/**
 * AJAX callback; loads namespace URI for a given workspace.
 * 
 * @param array $form
 * @param array $form_state
 * @return array Namespace form field.
 */
function geoserver_ui_admin_settings_form_namespace($form, $form_state) {
  
  $workspace  = $form['geoserver_workspace']['#value'];
  $namespace  = '';
  
  if (!empty($workspace)) {
      
    $result = geoserver_get('rest/namespaces/' . $workspace . '.json');
    if ($result->code == 200) {
      $namespace = $result->data->namespace->uri;
    }
  }
  
  $form['geoserver_namespace']['#value'] = $namespace;
  
  return array($form['geoserver_namespace']);
}

/**
 * Menu callback. Display the administration GUI of GeoServer in an iframe.
 */
function geoserver_ui_admin() {
  $url = variable_get('geoserver_url', '');
  return array('#markup' => '<iframe src="' . $url . '" width="100%" height="2000px" onload="jQuery(\'iframe\').contents().find(\'#header\').hide()"></iframe>');
}

