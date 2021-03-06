<?php
// $Id$

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
 * Contains CQL exceptions defines
 */

define("MEDIAMOSA_CQL_EXCEPTION_SEVERITY_HIGH", mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_HIGH);
define("MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL", mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_NORMAL);
define("MEDIAMOSA_CQL_EXCEPTION_SEVERITY_LOW", mediamosa_exception::MEDIAMOSA_EXCEPTION_SEVERITY_LOW);
define("MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NONE", 0);

/**
 * Default cql exception
 */
class mediamosa_cql_exception extends mediamosa_exception {

  public function __construct($s_message, $severity = MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL) {
    parent::__construct(mediamosa_error::ERRORCODE_CQL_ERROR, $s_message, $severity);
  }
}

/**
 * Thrown when the end of the CQL string is reached
 * Is not an error, used for catch purposes
 */
class mediamosa_cql_exception_eos extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('CQL: End of string reached'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NONE);
  }
}

/**
 * Thrown when ) was found.
 * (end of scope)
 */
class mediamosa_cql_exception_eo_scope extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('CQL: End of scope reached'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NONE);
  }
}

/**
 * Thrown when cql_exception_eo_scope was captured without
 * the matching ( to catch it.
 *
 */
class mediamosa_cql_exception_eo_scope_unmatched extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('CQL: Unmatched \')\' encountered'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when not matching quotes was found.
 */
class mediamosa_cql_exception_eo_quote_unmatched extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('CQL: Unmatched \'"\' encountered'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown if an illegal escape was done on a char
 *
 */
class mediamosa_cql_exception_invalid_escape_char extends mediamosa_cql_exception {

  public function __construct($char, $i_pos) {
    parent::__construct(mediamosa::t('CQL: Invalid escape char \'@char\' on position @pos.', array('@char' => $char, '@pos' => $i_pos)), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when string that was provided was empty...
 *
 */
class mediamosa_cql_exception_string_is_empty extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('CQL: An empty CQL string was provided'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when a character was expected
 *
 */
class mediamosa_cql_exception_expecting_char extends mediamosa_cql_exception {

  public function __construct($char, $i_pos) {
    parent::__construct(mediamosa::t('CQL: Syntax error, expecting @char on position @pos', array('@char' => $char, '@pos' => $i_pos)), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when we were still expecting some cql...
 *
 */
class mediamosa_cql_exception_unexpected_eos extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('Unexpected end of string'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * This exception is not used by CQL (yet), but top layers may use it to when detecting
 * wrong mixed booleans. Mixing booleans might not be logical, but is allowed by CQL.
 *
 */
class mediamosa_cql_exception_unlogic_boolean_usage extends mediamosa_cql_exception {

  public function __construct() {
    parent::__construct(mediamosa::t('You are mixing OR and AND booleans in the same scope, not allowed (e.g. bad; (expression AND expression OR expression), good; (expression AND (expression OR expression)) ).'), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}

/**
 * Thrown when syntax error was found
 *
 */
class mediamosa_cql_exception_syntax_error extends mediamosa_cql_exception {

  public function __construct($str_message, $i_pos) {
    parent::__construct(mediamosa::t('Syntax error at position @pos, @message', array('@pos' => $i_pos, '@message' => $str_message)), MEDIAMOSA_CQL_EXCEPTION_SEVERITY_NORMAL);
  }
}
