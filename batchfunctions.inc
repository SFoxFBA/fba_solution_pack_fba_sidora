<?php

function addlicencetoobject($pid, &$context) {
  module_load_include('inc', 'sidora', 'includes/utils');
  module_load_include('module', 'tuque_wrapper', 'tuque_wrapper');
  $path = drupal_get_path('module', 'fba_solution_pack_fba_sidora');
  $object = islandora_object_load($pid);
  $dsid = 'LICENCE';
  $label = 'Licence';
  $mimetype = 'text/html';
  sidora_update_datastream($object, $path . '/includes/licence1.txt', $dsid, $label, $mimetype);

  return TRUE;
}

