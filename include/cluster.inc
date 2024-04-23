<?php
class Cluster {
	private $topSlots;
	private $bottomSlots;
	private $width;
	private $slotLimit;
    private $tickCounter;
    private $tickAt;

	public function __construct($width = null, $slotLimit = null) {
        $this->tickCounter = 0;
        $this->tickAt = Config::$defaultTickAt;

		// initialize
        if (is_null($width)) {
		    $this->width = Config::$defaultWidth;
        } else {
            $this->width =$width;
        }

        if (is_null($slotLimit)) {
            $this->slotLimit = Config::$defaultHeight;
        } else {
            $this->slotLimit = $slotLimit;
        }

		$this->topSlots = array();
		$this->bottomSlots = array();

		for ($x = 0; $x < $this->width; $x++) {
			$this->topSlots[$x] = array();
			$this->bottomSlots[$x] = array();

			$count = rand(0, $this->slotLimit);

			for ($y = 0; $y < $count; $y++) {
				$this->topSlots[$x][] = new Fuzzy();
			}

			$count = rand(0, $this->slotLimit);

			for ($y = 0; $y < $count; $y++) {
				$this->bottomSlots[$x][] = new Fuzzy();
			}
		}	

		$this->draw();
		$this->gameLoop();
	}

	public function gameLoop() {
        $this->term = `stty -g`;
		system("stty -icanon -echo");

        stream_set_blocking(STDIN, false);

		while (1) {
			$draw = true;

			$c = Config::waitForInput();

			switch ($c) {
				case 'q':
					$this->gameOver();
				default:
			}
			
			$this->tickCounter++;

            if ($this->tickCounter >= $this->tickAt) {
                $this->tickCounter = 0;
                $this->tick();

                if ($draw) {
                    $this->draw();
                }
            }
		}
	}

    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

	public function draw() {
		$ret = array();

		// add those together, plus one for the middle line
		$height = $this->slotLimit + $this->slotLimit + 1;

		// do the top slots
		for ($x = 0; $x < $this->slotLimit; $x++) {
			for ($y = 0; $y < $this->width; $y++) {
				if (!isset($ret[$x])) {
					$ret[$x] = "";
				}

				$ts = count($this->topSlots[$y]);

				if ($ts >= ($this->slotLimit - $x)) {
					$ret[$x] .= "*";
				} else {
					$ret[$x] .= " ";
				}
			}
		}
		
		// draw the middle line
		for ($y = 0; $y < $this->width; $y++) {
			if (!isset($ret[$x])) {
				$ret[$x] = "";
			}

			$ret[$x] .= Config::$middleLineChar;
		}

		// draw the bottom slots
		for ($x = $this->slotLimit + 1; $x < $height; $x++) {
			for ($y = 0; $y < $this->width; $y++) {
				if (!isset($ret[$x])) {
					$ret[$x] = "";
				}

				$bs = count($this->bottomSlots[$y]);

				if ($bs > ($x - ($this->slotLimit + 1))) {
					$ret[$x] .= "*";
				} else {
					$ret[$x] .= " ";
				}
			}
		}

		// draw them
		Config::draw($ret);
	}

	public function tick() {
        // MOVE A RANDOM FUZZY
        do {
            $col = rand(0, $this->width - 1);
            $top = rand(0, 1);
            $fuzzy = null;

            if ($top) {
                $count = count($this->topSlots[$col]);
                
                if ($count > 0) {
                    $stack = rand(0, $count - 1);
                    $fuzzy = $this->topSlots[$col][$stack];
                }        
            } else {
                $count = count($this->bottomSlots[$col]);

                if ($count > 0) {
                    $stack = rand(0, $count - 1);
                    $fuzzy = $this->bottomSlots[$col][$stack];
                }
            }
        } while (is_null($fuzzy));

        $possibleMoves = array("NOWHERE");

        // check for available moves
        if ($col == 0) {
            $leftCol = $this->width - 1;
        } else {
            $leftCol = $col - 1;
        }

        if (count($this->topSlots[$leftCol]) < $this->slotLimit) {
            $possibleMoves[] = "LEFT";
        }

        // right
        if ($col == $this->width - 1) {
            $rightCol = 0;
        } else {
            $rightCol = $col + 1;
        }
				
        if (count($this->topSlots[$rightCol]) < $this->slotLimit) {
            $possibleMoves[] = "RIGHT";
        }

        // switch from top to bottom
        if (count($this->bottomSlots[$col]) < $this->slotLimit) {
            $possibleMoves[] = "SWITCH";
        }

        $move = $fuzzy->getMove($possibleMoves);

        switch ($move) {
            case 'LEFT':
                if ($top) {
                    $this->topSlots[$leftCol][] = $fuzzy;
                    $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                } else {
                    $this->bottomSlots[$leftCol][] = $fuzzy;
                    $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                }

                break;
            case 'RIGHT':
                if ($top) {
                    $this->topSlots[$rightCol][] = $fuzzy;
                    $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                } else {
                    $this->bottomSlots[$rightCol][] = $fuzzy;
                    $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                }

                break;
            case 'SWITCH':
                if ($top) {
                    $this->bottomSlots[$col][] = $fuzzy;
                    $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                } else {
                    $this->topSlots[$col][] = $fuzzy;
                    $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                }

                break;
            case 'NOWHERE':
                // do nothing

                break;
            default:
                echo "We have an unhandled getMove response\n";
                die();
        }
	}
}