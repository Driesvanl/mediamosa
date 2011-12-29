<?php
/**
 * MediaMosa is Open Source Software to build a Full Featured, Webservice
 * Oriented Media Management and Distribution platform (http://mediamosa.org)
 *
 * Copyright (C) 2011 SURFnet BV (http://www.surfnet.nl) and Kennisnet
 * (http://www.kennisnet.nl)
 *
 * MediaMosa is based on the open source Drupal platform and
 * was originally developed by Madcap BV (http://www.madcap.nl)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, you can find it at:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 */

/**
 * @file
 * Cmusphinx tool functions.
 */
class mediamosa_tool_cmusphinx {
  /**
   * Name / ID of the module.
   */
  const NAME = 'cmusphinx';
  const EXT = 'srt';

  /**
   * Get the exec string for transcoding the file.
   *
   * @deprecated: Although new, its deprecated. We will replace this function
   *              with transcode() very soon and no longer rely on exec
   *              strings return. Rewrite of jobs is planned for v3.5.
   *
   * @param array $options
   */
  static public function get_transcode_exec($options) {

    $params = " nbest_v7s.lexZ100_5_x1RT_eval.ini ";
    if (strstr($options['parameter_string'], '400k') != FALSE) {
      $params = " nbest_v7s.lexZ400_5_x3RT_eval2.ini ";
    }

    // handle seconds option: "-seconds 10"
    $limitduration = "";
    preg_match("/-seconds [0-9]+/", $options['parameter_string'], $matches);
    if ($matches) {
      $sec = (int) substr($matches[0], 9);
      if ($sec > 0) {
        $limitduration = " -t " . $sec; // " -t 30 ";
      }
    }

    // handle -subtitlefile option: "-subtitlefile yes"
    $subtitlefile = FALSE;
    preg_match("/-subtitlefile yes/", $options['parameter_string'], $matches);
    if ($matches) {
      $subtitlefile = TRUE;
    }
    
    if ($options['file_extension'] == 'srt') {
      // Build the values.
      $execution_string_options = array(
        '@file_location' => dirname($options['location_dest_file']),
        '@mediafile_location' => $options['location_source_file'],
        '@mediafile_id' => $options['mediafile_id'],
        '@params' => $params,
        '@limitduration' => $limitduration,
      );

      // Build the execute_string.
      $command = '
cd @file_location;
{ echo "Errors: none";
echo "Status: in progress";
echo "Progress: 0.01";
echo "Warnings: none"; } > @mediafile_id.status;

# step 1: make .wav file.
# debug: -t 10
nice ffmpeg -i @mediafile_location @limitduration -vn -acodec pcm_s16le -ar 16000 -y -ac 1 -f wav @mediafile_id.raw

# create ctlfile & cfgfile
echo "@mediafile_id" > @file_location/ctlfile

cd @file_location;

# start transcription.
sphinx3_livepretend ctlfile . /usr/src/sphinx/cfgfile &> /dev/stdout | tee aap
grep "^fv:" aap > aap2
cut -c 11-31,33-38,39-44 aap2 > @mediafile_id.srt

# clean up
# --todo
# rm @mediafile_id.res.*

# all done.
{ echo "Errors: none";
echo "Status: done";
echo "Progress: 1.000";
echo "Warnings: none"; } > @mediafile_id.status;';

      $exec_string = strtr($command, $execution_string_options);
      watchdog('AAA', $exec_string);
      return $exec_string;
    }

    return FALSE;
  }


  /**
   * Hook perform post transcode actions.
   *
   * This function stores the found transcription in metadata with the
   * asset. The named metadatafield is 'transcription'. Only 1
   * metadatafield with transcription is allowed per asset.
   *
   */
  static public function post_transcode($asset_id, $mediafile_id, $mediafile_id_dest) {

    // Get the filename on storage.
    $file_transcode = mediamosa_configuration_storage::mediafile_filename_get(array('mediafile_id' => $mediafile_id_dest));

    // Read the .srt file.
    $transcript = file_get_contents($file_transcode);

    // Store it under the metadatafield 'transcript' under the asset.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('transcript'));
    mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, array('transcript' => array($transcript)), 'update');

    // and tag the file for playing purposes;
    mediamosa_asset_mediafile::update_mediafile_tag($mediafile_id_dest, 'srt');

    return TRUE;
  }
}
