<?php

function bulk_display_drush_command() {
  $commands = array();

  $commands['bulk-display-show'] = array(
    'callback' => 'bulk_display_callback',
    'callback arguments' => array('show'),
    'description' => 'Show all fields for an entity\'s view mode',
    'aliases' => array('bds'),
    'examples' => array(
      'drush bds [view_mode] [bundle] [entity_type]' => 'Show all fields for an entity\'s view mode',
    ),
    'arguments' => array(
      'view_mode' => 'View mode machine name',
      'bundle' => 'Bundle name',
      'entity_type' => 'The entity type. Defaults to node',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $commands['bulk-display-hide'] = array(
    'callback' => 'bulk_display_callback',
    'callback arguments' => array('hide'),
    'description' => 'Hide all fields for an entity\'s view mode',
    'aliases' => array('bdh'),
    'examples' => array(
      'drush bds [view_mode] [bundle] [entity_type]' => 'Hide all fields for an entity\'s view mode',
    ),
    'arguments' => array(
      'view_mode' => 'View mode machine name',
      'bundle' => 'Bundle name',
      'entity_type' => 'The entity type. Defaults to node',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $commands;
}

/**
 * Change all field display settings.
 */
function bulk_display_callback($action, $view_mode, $bundle, $entity_type = 'node') {
  // verify the action
  if (!in_array($action, array('show', 'hide'))) {
    drush_log(t('Invalid action: !action', array('!action' => $action)), 'error');
    return FALSE;
  }

  // verify the arguments
  $entity_info = entity_get_info($entity_type);
  if (!$entity_info) {
    drush_log(t('Invalid entity type: !entity_type', array('!entity_type' => $entity_type)), 'error');
    return FALSE;
  }
  if (!isset($entity_info['view modes'][$view_mode])) {
    drush_log(t('Invalid view mode for !entity_type: !view_mode', array('!entity_type' => $entity_type, '!view_mode' => $view_mode)), 'error');
    $available_view_modes = array_keys($entity_info['view modes']);
    drush_log(t('Available view modes: !view_modes', array('!view_modes' => implode(', ', $available_view_modes))), 'ok');
    return FALSE;
  }
  if (!isset($entity_info['bundles'][$bundle])) {
    drush_log(t('Invalid bundle for !entity_type: !bundle', array('!entity_type' => $entity_type, '!view_mode' => $view_mode)), 'error');
    return FALSE;
  }

  if ($view_mode === 'full') {
    $view_mode = 'default';
  }

  $field_info = field_info_instances($entity_type, $bundle);
  foreach (array_keys($field_info) as $field_name) {
    $field_config = field_read_instance($entity_type, $field_name, $bundle);

    // check that this instance has settings for a given type
    if (!isset($field_config['display'][$view_mode])) {
      drush_log(t('Cannot change field settings for view mode: !field_name is not setup for !view_mode', array(
        '!field_name' => $field_name,
        '!view_mode' => $view_mode
      )), 'error');
      return FALSE;
    }

    // change the configuration for the action
    if ($action === 'hide') {
      $field_config['display'][$view_mode]['type'] = 'hidden';
    }
    elseif ($action === 'show') {
      // show is a bit more tricky because we need to set to the default
      // formatter based on the field type
      $field_field = field_info_field($field_name);
      $field_type_info = field_info_field_types($field_field['type']);
      $field_config['display'][$view_mode]['type'] = $field_type_info['default_formatter'];
      $field_config['display'][$view_mode]['module'] = $field_type_info['module'];
    }

    // var_dump($field_config);
    field_update_instance($field_config);
  }
}
