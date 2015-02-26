<?php

/**
 * Implements hook_form().
 *
 * Landing page for import function
 *
 * @return string
 *   html formatted output
 */
function fba_solution_pack_fba_sidora_form_publish(&$form_state = array()) {
  // $form_state['storage']['step'] keeps track of what page we're on.
  if (!isset($form_state['storage']['step'])) {
    $form_state['storage']['step'] = 1;
  }

  //Don't lose our old data when returning to a page with data already typed in.
  $default_value = '';
  if (isset($form_state['storage']['values'][$form_state['storage']['step']])) {
    $default_value = $form_state['storage']['values'][$form_state['storage']['step']];
  }

  switch ($form_state['storage']['step']) {
    case 1:
      $getpath = isset($_GET['q']) ? $_GET['q'] : '';
      $lastslash = strrpos($getpath, '/');
      $pid = substr($getpath, $lastslash + 1);

      module_load_include('inc', 'sidora', 'includes/utils');
      $currentrels = array();
      $repository = tuque_wrapper_get_repository_instance();
      $objecttopublish = $repository->getObject($pid);
      //SFOX maybe use this? $relationships = $objecttopublish->getParents();
      $relationships = $objecttopublish->relationships->get('info:fedora/fedora-system:def/relations-external#', 'isMemberOfCollection');
      foreach ($relationships as $relationship) {
        $currentrels[$relationship[object][value]] = $relationship[object][value];
      }
      $futurerels = $this->getPublishCollectionsList();
      $remainingrels = array_diff($futurerels, $currentrels); //Don't want to try and re-add it to an existing collection
      foreach ($remainingrels as $rel) {
        $relsarray .= '<option value=' . $rel . '>' . $rel . '</option>';
      }

      $form = array();
      // Set the form encoding type.
      $form['#attributes']['enctype'] = "multipart/form-data";

      $currentrelstext = '<br/>' . $pid . ' currently in collection(s) <br/>';
      if ($currentrels) {
        foreach ($currentrels as $currentrel) {
          $currentrelstext .= ' - ' . $currentrel;
        }
      }
      else {
        $currentrelstext .= ' - none';
      }
      $currentrelstext .= '<br/>';

      $form['currentcollections'] = array(
        '#type' => 'markup',
        '#value' => $currentrelstext,
      );
      $form['publishcollections'] = array(
        '#type' => 'select',
        '#title' => t('Publish to collection'),
        '#options' => $remainingrels,
      );
      $form['pidtopublish'] = array(
        '#type' => 'hidden',
        '#value' => $pid,
      );

      break;
    case 2:
      $licenceprivacytext = 'Some stuff will go here to explain licensing. <br/><br/>';
      $selectedcollection = $form_state[storage][values][1][publishcollections];
      $selectedpid = $form_state[storage][values][1][pidtopublish];
      $licenceprivacytext .= $selectedpid . ' has been chosen for publishing to ' . $selectedcollection . '.<br/><br/>';
      $form['licenceprivacy'] = array(
        '#type' => 'markup',
        '#value' => $licenceprivacytext,
      );
      $form['selectedcollection'] = array(
        '#type' => 'hidden',
        '#value' => $selectedcollection,
      );
      $form['selectedpid'] = array(
        '#type' => 'hidden',
        '#value' => $selectedpid,
      );
      break;
  }

  //Depending on what page we're on, show the appropriate buttons.
  if ($form_state['storage']['step'] > 1) {
    $form['previous'] = array(
      '#type' => 'submit',
      '#value' => t('<< Previous'),
    );
  }
  if ($form_state['storage']['step'] != 2) {
    $form['next'] = array(
      '#type' => 'submit',
      '#value' => t('Continue >>'),
    );
  }
  else {
    $form['finish'] = array(
      '#type' => 'submit',
      '#value' => t('Publish Object'),
    );
  }
  return $form;
}

/** RDF Relations section * */

/**
 * Implements hook_submit.
 * Return status message of autocomplete path and title strings.
 *
 * @param array $form
 *   standard form variable for a hook
 * @param array $form_state
 *   standard form state variable for a hook
 */
function fba_solution_pack_fba_sidora_form_publish_submit($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');

  //Save the values for the current step into the storage array.
  $form_state['storage']['values'][$form_state['storage']['step']] = $form_state['values'];

  //Check the button that was clicked and change the step.
  if ($form_state['clicked_button']['#id'] == 'edit-previous') {
    $form_state['storage']['step']--;
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-next') {
    $form_state['storage']['step']++;
  }
  elseif ($form_state['clicked_button']['#id'] == 'edit-finish') {
    //Add the relsext here.
    $selectedpid = $form_state[storage][values][2][selectedpid];
    $selectedcollection = $form_state[storage][values][2][selectedcollection];

    $repository = tuque_wrapper_get_repository_instance();
    $objecttopublish = $repository->getObject($selectedpid);
    $objecttopublish->relationships->add(RELS_EXT_URI, 'isMemberOfCollection', $selectedcollection);

    //We must do this or the form will rebuild instead of refreshing.
    unset($form_state['storage']);

    //Go to this page after completing the form.
    $form_state['redirect'] = 'fbasidora/publish/complete';
  }
}

/**
 *
 * @param type $form
 * @param type $form_state
 */
function fba_solution_pack_fba_sidora_form_publish_validate($form, &$form_state) {
  $selectedcollection = (isset($form_state['values']['publishcollections'])) ? $form_state['values']['publishcollections'] : '';
}

function dataset_published(&$form_state = array()) {
  //put some text on screen from here?
}
