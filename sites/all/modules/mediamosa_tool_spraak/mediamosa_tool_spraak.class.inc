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
 * Spraak tool functions.
 */
class mediamosa_tool_spraak {
  /**
   * Name / ID of the module.
   */
  const NAME = 'spraak';
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
nice ffmpeg -i @mediafile_location @limitduration -vn -acodec pcm_s16le -ar 16000 -y -ac 1 -f wav @mediafile_id.wav

# start transcription.
export SPR_HOME=/opt/spraak/spraak_v1.0.346/
export OPSYS=Linux_x86_64
export PATH=$PATH:$SPR_HOME/bin:$SPR_HOME/bin/$OPSYS
cd /opt/spraak/nbest
nice ./GOnbest @params @file_location/@mediafile_id.wav @file_location/@mediafile_id.res

cd @file_location;

# final step: get the words
cut -d \' \' -f 3,5 @mediafile_id.res.ctm_pp | grep -v \' uh\$\' > @mediafile_id.srt

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
   * Convert a voice output to spr file.
   *
   * @param string $transcript_file
   *   The filename of the subtitles.
   * @param array $options
   *   Options;
   *     - maxlinelength
   *       The maximum length of each line.
   */
   static public function convert_spr_to_srt($transcript_file, $options = array()) {
    /*
      input format:
      0.000 op
      0.740 grote
      1.140 leermeester
      1.740 van
      1.920 maïs
      2.300 geleefd
      2.740 en
      2.950 wanten
      3.460 .
      4.120 ik
      4.320 ben
      5.300 ontelbare
      6.140 malen
      7.850 in
      7.980 dat
      9.060 landhuizen
      9.580 Amsterdam

      output in srt format with 7 words per line:
       1
       00:00:00,000 --> 00:00:02,740
       Grote leermeester voor mij is geweest Rembrandt

       2
       00:00:02,740 --> 00:00:06,140
       Ik ben ontelbare malen

       3
       00:00:06,140 --> 00:00:10,750
       in het Rembrandthuis in amsterdam gaan studeren
      */

     $options += array(
       'maxlinelength' => 35,
       'skip_words'=> array('ggg', '<unk>', 'uh', '<PARTIAL>'),
     );

    $handle = @fopen($transcript_file, 'r');
    $srt = array();

    if ($handle) {
      $line = '';
      $end = '';
      $start = '';
      $ucfirst = TRUE;

      // Get one line of the text.
      while (($buffer = fgets($handle, 4096)) !== FALSE) {
        // Split up the time and text.
        list($seconds, $milliseconds, $word) = sscanf($buffer, '%d.%d %s');
        $word_time = sprintf('%02d:%02d:%02d,%03d', floor($seconds / 3600), floor($seconds / 60), ($seconds % 60), $milliseconds);

        // Some error words we need to skip.
        if (in_array($word, $options['skip_words'])) {
          continue;
        }

        // Save the start time when not set yet.
        if (empty($start)) {
          $end = $start = $word_time;
        }

        // Build up line.
        $line .= $word == '.' ? '.' : (empty($line) ? '' : ' ') . ($ucfirst ? ucfirst($word) : $word);

        // Remember when previous was a dot.
        $ucfirst = $word == '.';

        // Is the line to long?
        if (drupal_strlen($line) > $options['maxlinelength']) {
          // Every 3 a time line.
          $number = floor(count($srt) / 3);
          if ($number * 3 == count($srt)) {
            $srt[] = $number + 1 . "\n". $start . ' --> ' . $end;
            $start = $word_time;
          }

          $line_truncated = truncate_utf8($line, $options['maxlinelength'], TRUE);
          $line = trim(drupal_substr($line, drupal_strlen($line_truncated)));
          $srt[] = $line_truncated;
          $srt[] = '';
        }

        // Set end to last word.
        $end = $word_time;
      }

      if (!empty($line)) {
        // Every 3 a time line.
        $number = floor(count($srt) / 3);
        if ( ($number * 3) == count($srt)) {
          $srt[] = $number + 1 . "\n". $start . ' --> ' . $end;
        }

        $line_truncated = truncate_utf8($line, $options['maxlinelength']);
        $line = trim(drupal_substr($line, drupal_strlen($line_truncated)));
        $srt[] = $line_truncated;
      }

      // Close the source file.
      fclose($handle);
    }

    // Overwrite possible srt file.
    file_put_contents($transcript_file, implode("\n", $srt));
  }


  /**
   * Convert a spraak output file to timed formatted string.
   *
   */
  static public function convert_spr_to_timed($transcript) {
    /*
    input format:
      0.000 op
      0.740 grote
      1.140 leermeester
      1.740 van
      1.920 maïs
      2.300 geleefd
      2.740 en
      2.950 wanten
      3.460 .
      4.120 ik
      4.320 ben
      5.300 ontelbare
      6.140 malen
      7.850 in
      7.980 dat
      9.060 landhuizen
      9.580 Amsterdam
    output time formatted code without: dots
      0:00 op
      0:00 grote
      0:01 leermeester
      0:01 van
      0:01 maïs
      0:02 geleefd
      0:02 en
      0:02 wanten
      0:04 ik
      0:04 ben
      0:05 ontelbare
      0:06 malen
      0:07 in
      0:07 dat
      0:09 landhuizen
      0:09 Amsterdam
    */
    $formatted_transcript = '';

    $lijst = explode("\n", $transcript);
    foreach ($lijst as $regel) {
      $seconds = (int) trim($regel);
      $lastword = substr(strrchr($regel, ' '), 1);
      if ($seconds > 3600) {
        $formatted_transcript .= date('H:i:s', $seconds) . ' ' . $lastword . "\n";
      }
      else {
        $formatted_transcript .= date('i:s', $seconds) . ' ' . $lastword . "\n";
      }
    }
    return $formatted_transcript;
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

    $transcript = self::convert_spr_to_timed($transcript);
    
    // Store it under the metadatafield 'transcript' under the asset.
    $metadata_definitions_full = mediamosa_asset_metadata_property::get_metadata_properties_full(FALSE, array('transcript'));
    mediamosa_asset_metadata::metadata_create($asset_id, $metadata_definitions_full, array('transcript' => array($transcript)), 'update');

    // for searching this is good, however: the .srt has to be a specific format.
    self::convert_spr_to_srt($file_transcode);

    // and tag the file for playing purposes;
    mediamosa_asset_mediafile::update_mediafile_tag($mediafile_id_dest, 'srt');

    return TRUE;
  }
}
