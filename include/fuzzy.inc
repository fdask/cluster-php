<?php
class Fuzzy {
	// the fuzzies penchants for doing which moves
    private $goLeft;
	private $goRight;
	private $goNowhere;
	private $goSwitch;

    public static $moveLeft = "LEFT";
    public static $moveRight = "RIGHT";
    public static $moveNowhere = "NOWHERE";
    public static $moveSwitch = "SWITCH";

	public function __construct() {
		$pts = 100;

		$this->goLeft = rand(0, $pts);
		$this->goRight = rand(0, $pts - $this->goLeft);
		$this->goNowhere = rand(0, $pts - $this->goLeft - $this->goRight);
		$this->goSwitch = $pts - ($this->goLeft + $this->goRight + $this->goNowhere);
	}

    public function tick() {

    }

    public function getMove($possibleMoves) {
        $gl = $this->goLeft;
        $gr = $this->goRight;
        $gn = $this->goNowhere;
        $gs = $this->goSwitch;

        do {
            $i = rand(0, 100);
            
            // 0 > gl > gl + gr > gl + gr + gn > 100 - (gl + gn + gr)
            if ($i < $gl ) {
                // left
                $move = Fuzzy::$moveLeft;
            } else if ($i >= $gl && $i < ($gl + $gr)) {
                // go right
                $move = Fuzzy::$moveRight;
            } else if ($i >= ($gl + $gr) && $i < ($gl + $gr + $gn)) {
                // go nowhere
                $move = Fuzzy::$moveNowhere;
            } else if ($i >= ($gl + $gr + $gn)) {
                // go switch
                $move = Fuzzy::$moveSwitch;
            }
        } while (!in_array($move, $possibleMoves));

        return $move;
    }
}