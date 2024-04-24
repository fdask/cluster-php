<?php
class Cluster {
	private $topSlots;
	private $bottomSlots;
	private $width;
	private $slotLimit;
    
    private $tickCounter;
    private $tickEnable;
    private $tickTotal;

    private $bgColor;
    private $concurrentMoves;
    private $trails;

	public function __construct($width = null, $slotLimit = null) {
        $this->concurrentMoves = 1; //Config::$defaultConcurrentMoves;
        $this->bgColor = Config::$defaultBGColor;
    
        $this->tickCounter = 0;
        $this->tickEnable = true;
        $this->tickTotal = Config::$defaultTickTotal;
    
        $this->trails = false;

		// initialize
        if (is_null($width)) {
		    $this->width = Config::$defaultWidth;
        } else {
            $this->width = $width;
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

        // START INFINITE LOOP
		while (1) {
			$draw = true;

			$c = Config::waitForInput();

			switch ($c) {
				case 'q':
					$this->gameOver();

                    break;
                case '1':
                case '2':
                case '3':
                case '4':
                    // pick a random color
                    $color = array_rand(Colors::$foreground);

                    // send a tick to all the fuzzies
                    for ($x = 0; $x < count($this->topSlots); $x++) {
                        for ($y = 0; $y < count($this->topSlots[$x]); $y++) {
                            if (!is_null($this->topSlots[$x][$y])) {
                                if (rand(0, $c - 1) === $c - 1) {
                                    $this->topSlots[$x][$y]->setColor($color, rand(2, 10));
                                }
                            }
                        }
                    }

                    for ($x = 0; $x < count($this->bottomSlots); $x++) {
                        for ($y = 0; $y < count($this->bottomSlots[$x]); $y++) {
                            if (!is_null($this->bottomSlots[$x][$y])) {
                                if (rand(0, $c - 1) === $c - 1) {
                                    $this->bottomSlots[$x][$y]->setColor($color, rand(2, 10));
                                }
                            }
                        }
                    }

                    $this->draw();

                    break;
                case 'b':
                    $this->bgColor = array_rand(Colors::$background);
                    
                    $this->draw();

                    break;
                case 't':
                    $this->trails = ($this->trails ? false : true);

                    break;
                case 'T':
                    $this->tickEnable = ($this->tickEnable ? false : true);
                
                    break;
				default:
                    // do nothing here
			}
			
			$this->tickCounter++;

            if ($this->tickCounter >= $this->tickTotal) {
                $this->tickCounter = 0;

                if ($this->tickEnable) {
                    $this->tick();
                }

                if ($draw && $this->tickEnable) {
                    $this->draw();
                }
            }
		}
        // END OF INFINITE LOOP
	}

    public function gameOver() {
        system("stty " . $this->term);

        exit;
    }

    public function tick() {
        // send a tick to all the fuzzies
        for ($x = 0; $x < count($this->topSlots); $x++) {
            for ($y = 0; $y < count($this->topSlots[$x]); $y++) {
                if (!is_null($this->topSlots[$x][$y])) {
                    $this->topSlots[$x][$y]->tick();
                }
            }
        }

        for ($x = 0; $x < count($this->bottomSlots); $x++) {
            for ($y = 0; $y < count($this->bottomSlots[$x]); $y++) {
                if (!is_null($this->bottomSlots[$x][$y])) {
                    $this->bottomSlots[$x][$y]->tick();
                }
            }
        }

        for ($cm = $this->concurrentMoves; $cm > 0; $cm--) {
            // select a random fuzzy
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

            $possibleMoves = array(Fuzzy::$moveNowhere);

            // check for available moves
            if ($col == 0) {
                $leftCol = $this->width - 1;
            } else {
                $leftCol = $col - 1;
            }

            if (count($this->topSlots[$leftCol]) < $this->slotLimit) {
                $possibleMoves[] = Fuzzy::$moveLeft;
            }

            // right
            if ($col == $this->width - 1) {
                $rightCol = 0;
            } else {
                $rightCol = $col + 1;
            }
                    
            if (count($this->topSlots[$rightCol]) < $this->slotLimit) {
                $possibleMoves[] = Fuzzy::$moveRight;
            }

            // switch from top to bottom
            if (count($this->bottomSlots[$col]) < $this->slotLimit) {
                $possibleMoves[] = Fuzzy::$moveSwitch;
            }

            $move = $fuzzy->getMove($possibleMoves);

            // set a color for the fuzzy if trails are on
            if ($this->trails) {
                $color = array_rand(Colors::$foreground);
                
                if ($move != Fuzzy::$moveNowhere) {
                    $fuzzy->setColor($color, rand(2, 10));
                }
            }

            switch ($move) {
                case Fuzzy::$moveLeft:
                    if ($top) {
                        if (Config::$debug) echo "Moving top col: $col, stack: $stack to col: $leftCol\n";
                        $this->topSlots[$leftCol][] = $fuzzy;
                        $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                    } else {
                        if (Config::$debug) echo "Moving bottom col: $col, stack: $stack to col: $leftCol\n";
                        $this->bottomSlots[$leftCol][] = $fuzzy;
                        $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                    }

                    break;
                case Fuzzy::$moveRight:
                    if ($top) {
                        if (Config::$debug) echo "Moving top col: $col, stack: $stack to col: $rightCol\n";
                        $this->topSlots[$rightCol][] = $fuzzy;
                        $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                    } else {
                        if (Config::$debug) echo "Moving bottom col: $col, stack: $stack to col: $rightCol\n";
                        $this->bottomSlots[$rightCol][] = $fuzzy;
                        $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                    }

                    break;
                case Fuzzy::$moveSwitch:
                    if ($top) {
                        if (Config::$debug) echo "Moving top col: $col, stack: $stack to bottom col: $col\n";
                        $this->bottomSlots[$col][] = $fuzzy;
                        $this->topSlots[$col] = Config::removeIndex($this->topSlots[$col], $stack);
                    } else {
                        if (Config::$debug) echo "Moving bottom col: $col, stack: $stack to top col: $col\n";
                        $this->topSlots[$col][] = $fuzzy;
                        $this->bottomSlots[$col] = Config::removeIndex($this->bottomSlots[$col], $stack);
                    }

                    break;
                case Fuzzy::$moveNowhere:
                    // do nothing

                    break;
                default:
                    echo "We have an unhandled getMove response\n";
                    die();
            }
        }
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

                // how high is this column
				$ts = count($this->topSlots[$y]);

				if ($ts >= ($this->slotLimit - $x)) {
                    $ret[$x] .= $this->topSlots[$y][$this->slotLimit - $x - 1]->draw();
				} else {
					$ret[$x] .= Colors::bg_color($this->bgColor, " ");
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
					$ret[$x] .= $this->bottomSlots[$y][$x - ($this->slotLimit + 1)]->draw();
				} else {
					$ret[$x] .= Colors::bg_color($this->bgColor, " ");
				}
			}
		}

		// draw them
		Config::draw($ret);
	}
}