<?php
class Config {
	public static $debug = true;
	public static $defaultWidth = 80;
	public static $defaultHeight = 5;
	public static $defaultTickTotal = 4000;
	public static $defaultBGColor = 'black';
    public static $defaultConcurrentMoves = 1;
	public static $middleLineChar = "-";

	public static function draw($display) {
        if (!Config::$debug) {
            system("clear");
        }

        for ($x = 0; $x < count($display); $x++) {
            if (isset($display[$x])) {
                echo $display[$x];
            }
             
            echo "\n";
        }

        echo "\n";
    }

	public static function waitForInput() {
        $input = '';

        $read = [STDIN];
        $write = null;
        $except = null;

        readline_callback_handler_install('', function() {});

        // Read characters from the command line one at a time until there aren't any more to read
        do {
            $input .= fgetc(STDIN);
        } while (stream_select($read, $write, $except, 0, 1));

        readline_callback_handler_remove();

        return $input;
    }

	public static function autoload($class) {
        $file = __DIR__ . "/" . str_replace('\\', '/', strtolower($class)) . ".inc";
        
        if (file_exists($file)) {
            require $file;
        } else {
            if (Config::$debug) {
                debug_print_backtrace();
            }
            
            die("Couldnt find file: $file\n");
        }
    }

	public static function removeIndex($array, $i) {
		$ret = array();

		for ($x = 0; $x < count($array); $x++) {
			if (isset($array[$x])) {
				if ($x != $i) {
					$ret[] = $array[$x];
				}
			}
		}

		return $ret;
	}
}

spl_autoload_register("Config::autoload");