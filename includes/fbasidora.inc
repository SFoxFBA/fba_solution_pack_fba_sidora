<?php

/**
 *
 *
 */
function sidora_list_users() {
  $allrows = array();
  $solrresult = _sidora_perform_solr_search('PID:fba-user*');

  if ($solrresult) { //need to catch if restricted characters have been typed in
    $rawresponse = $solrresult->getRawResponse();
    $resarray = json_decode($rawresponse, TRUE);
    $resdocs = $resarray['response']['docs'];

    foreach ($resdocs as $resdoc) {
      $thispid = $resdoc[PID];
      $allrows[$thispid]['MSName'] = $resdoc[ownerId];
      $allrows[$thispid]['pid'] = $resdoc[PID];
      $allrows[$thispid]['MSStat'] = $resdoc[state];
      $usercollectionns = str_replace(array('-', ':'), '', $thispid);
      $allrows[$thispid]['privcollection'] = $usercollectionns;

      //$output .= $resdoc[PID] . ' : ' . $resdoc[state] . ' : ' . $resdoc[ownerId];
    }
  }

  //part 2 get mysql results
  $result = db_query("SELECT uid, name,data,status FROM {users} WHERE status = 1");
  while ($a_list = db_fetch_object($result)) {
    $uid = $a_list->uid;
    if ($uid > 1) { //no need to output anything for administrator
      foreach ($a_list as $array => $a) {
        switch ($array) {
          case 'uid': {
              break; //don't need to display this
            }
          case 'data': {
              $data = unserialize($a);
              $iup = $data['islandora_user_pid'];
              if (is_null($iup)) {
                $outputdata = "No PID found";
              }
              else {
                $outputdata = "PID = " . $iup;
              }
              break;
            }
          case 'name': {
              $outputname = $a; //$array . " = " .
            }
        }
      }
      $allrows[$iup]['IName'] = $outputname;
      $allrows[$iup]['pid'] = $iup;
    }
  }
  //part 3 get fbauser collections
  $solrresult = _sidora_perform_solr_search('PID:fbauser*');

  if ($solrresult) { //need to catch if restricted characters have been typed in
    $rawresponse = $solrresult->getRawResponse();
    $resarray = json_decode($rawresponse, TRUE);
    $resdocs = $resarray['response']['docs'];

    foreach ($resdocs as $resdoc) {
      $thispid = $resdoc[PID];
      $colonpos = strpos($thispid, ':');
      $thisns = substr($thispid, 0, $colonpos);
      $rowfound = FALSE;
      foreach ($allrows as $rowpid => $row) {
        if (strcmp($row['privcollection'], $thisns) == 0) {
          $rowfound = TRUE;
          $allrows[$rowpid]['privcollectionname'] = $thispid;
        }
      }
      if (!$rowfound) {
        $allrows[] = array('privcollectionname' => $thispid);
      }
    }
  }

  //part 4 see who's mentioned in fba:user-projects
  module_load_include('inc', 'sidora', 'includes/utils');
  $repository = tuque_wrapper_get_repository_instance();
  $userprojectobject = $repository->getObject('fba:user-projects');
  $userprojrels = $userprojectobject->relationships->get('info:fedora/fedora-system:def/relations-external#', 'hasConcept');
  foreach ($userprojrels as $userprojrel) {
    $destpid = $userprojrel['object']['value'];
    $allrows[$destpid]['isinuserproj'] = 'Yes';
  }

  //write out the table

  $colheadings = array('Islandora Name', 'PID', 'MySQL Name', 'MySQL Status', 'Private Collection name', 'Is in fba:user-project');

  foreach ($allrows as $row) {
    $tableoutput.= '<tr>';
    if ($row['IName']) {
      $iname = $row['IName'];
    }
    else {
      $iname = '(deleted user)';
    }
    $tableoutput .= '<td>' . $iname . '</td>';
    $tableoutput .= '<td>' . $row['pid'] . '</td>';
    $tableoutput .= '<td>' . $row['MSName'] . '</td>';
    $tableoutput .= '<td>' . $row['MSStat'] . '</td>';
    $tableoutput .= '<td>' . $row['privcollectionname'] . '</td>';
    $tableoutput .= '<td>' . $row['isinuserproj'] . '</td>';
    $tableoutput .= '</tr>';
  }

  $output .= '<table width="100%">';
  $output .= '<tr>';
  foreach ($colheadings as $coltext) {
    $output .= '<th>' . $coltext . '</th>';
  }
  $output .= '</tr>';
  $output .= $tableoutput;
  $output .= '</table>';
  $output .= '<br/> WHAT SHOULD HAPPEN WHEN THE DRUPAL USER HAS BEEN DELETED???';
  return $output;
}

/**
 * Blatantly ripped from apachesolr_views_query function build.
 * NB assumes solr is on localhost.
 *
 * @param string $queryparam
 *   to be passed within formatted solr query as 'q'
 *
 * @return array
 *   listing all appropriate results
 */
function _sidora_perform_solr_search($queryparam = 'dummy') {
  // TODO: needs tidying.
  $params = array();
  $params['rows'] = 40;
  $params['start'] = 0;
  $params['indent'] = 'on';
  $params['q'] = $queryparam;
  $params['version'] = '1.2';

  $params['fl'] = "*";
  $params['qt'] = "standard";
  $params['wt'] = "json";
  //  $params['hl'] = "true";
  //  $params['hl.fl'] = "dc.type";
  // If we display all items without pager.
  // Add filters to parameters.
  if (isset($params['filters'])) {
    $params['fq'] = $params['filters'];
  }

  // Add sorting.
  if (isset($params['sort'])) {
    // Populate sorting parameters.
    foreach ($params['sort'] as $field => $order) {
      $params['sort'][] = $field . ' ' . $order;
    }
  }

  // Set query.
  $dismax = NULL;
  if (isset($params['q'])) {
    // Query.
    $query = $params['q'];

    // Dismax.
    if (isset($params['defType']) AND $params['defType'] == 'dismax') {
      $dismax = $params['defType'];
      // If no qf fields are specified in the requestHandler
      // a default list is supplied here for dismax searches.
      $params['qf'] = 'dc.title^5 dc.subject^2 dc.description^2 dc.creator^2 dc.contributor^1 dc.type';
    }
  }
  else {
    $query = '*:*';
  }

  $url = variable_get('islandora_solr_search_block_url', 'http://localhost:8080/solr');
  $pathparts = parse_url($url);

  $solr = new Apache_Solr_Service($pathparts['host'], $pathparts['port'], $pathparts['path'] . '/');
  $solr->setCreateDocuments(0);

  // This is where the query gets executed and output starts being created.
  try {
    $results = $solr->search($query, $params['start'], $params['rows'], $params);
  }
  catch (Exception $e) {
    drupal_set_message(t('error searching ') . $e->getMessage());
  }
  return $results;
}
