<?php

function addlicensetoobject($pid, &$context) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'sidora', 'includes/utils');
  module_load_include('inc', 'fedora_repository', 'api/dublin_core');
  $path = drupal_get_path('module', 'fba_solution_pack_fba_sidora');
  $item = new Fedora_Item($pid);
  if ($item->exists()) {
    $dc = new Dublin_Core($item);
    $licensetext = file_get_contents($path. '/includes/license1.txt');
    $dc->set_element('dc:rights', array($licensetext));
    $dc->save();
  }
  return TRUE;
}