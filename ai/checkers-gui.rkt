#lang racket
(require racket/gui)

;; définitions modifiables pour configurer l'affichage
(define SCALE 90)            ; taille en pixel des cases
(define man-size (* SCALE 2/3)); taille des pions dessinés
(define king-size (/ SCALE 3)); taille des dames
(define black-color "Red")   ; couleur des pions du joueur "noir"
(define white-color "White") ; couleur des pions du joueur "blanc"
(define square-color "Green"); couleur des cases utilisées
(define empty-color "White") ; couleur des cases non utilisées

;; on représente les pions par des bits séparés
(define EMPTY                0)
(define BLACK                1)
(define WHITE                2)
(define KING                 4)
(define BLACK-KING           (bitwise-ior BLACK KING))
(define WHITE-KING           (bitwise-ior WHITE KING))
(define OUTSIDE              8)
(define COLOR-MASK           (bitwise-ior BLACK WHITE))
(define (color piece)        (bitwise-and piece COLOR-MASK))
(define (opponent-color a)   (bitwise-xor a COLOR-MASK))
(define (other-color? a b)   (= COLOR-MASK (bitwise-ior (color a) (color b))))
(define (same-color? a b)    (= (color a) (color b)))
(define (man? piece)         (not (= EMPTY (color piece))))
(define (king? piece)        (not (zero? (bitwise-and piece KING))))
(define (promote-king piece) (bitwise-ior piece KING))

;; quelques fonctions pour convertir la notation officielle
;; en coordonnées (x,y), et vice-versa
(define (position x y)       (+ 1 (quotient x 2) (* 4 y)))
(define (coordX pos)         (+ (* 2 (modulo (sub1 pos) 4))
                                1 (- (modulo (coordY pos) 2))))
(define (coordY pos)         (quotient (sub1 pos) 4))


;; quelques fonctions pour accéder au damier, représenté par un vecteur de vecteurs
(define board                (for/vector ([_ 8]) (make-vector 8 EMPTY)))
(define (board-ref x y)      (cond [(and (< -1 x 8) (< -1 y 8)) (vector-ref (vector-ref board y) x)]
                                   [else OUTSIDE]))
(define (board-set! x y val) (vector-set! (vector-ref board y) x val))
(define (get-piece pos)      (board-ref (coordX pos) (coordY pos)))
(define (white? pos)         (same-color? WHITE (get-piece pos)))
(define (black? pos)         (same-color? BLACK (get-piece pos)))
(define (set-piece! pos val) (board-set! (coordX pos) (coordY pos) val)
                             (draw-square pos dc))
(define (no-piece? x y)      (= EMPTY (board-ref x y)))
(define (inclusive-range a b)(range a (add1 b)))


(define (pos-between from to)
  (define dirX  (sgn (- (coordX to) (coordX from))))
  (define dirY  (sgn (- (coordY to) (coordY from))))
  (position (+ (coordX from) dirX) (+ (coordY from) dirY)))


;; dessin d'une case du plateau de jeu dans un contexte graphique dc
(define (draw-square pos dc)
  (define x (coordX pos))
  (define y (coordY pos))
  (define xOrg (* SCALE x))
  (define yOrg (* SCALE y))
  (define numbered? (not (zero? (modulo (+ x y) 2))))
  (send dc set-brush
           (if numbered? square-color empty-color)
           'solid)
  (send dc draw-rectangle xOrg yOrg SCALE SCALE)
  (when numbered?
    (send dc draw-text
             (number->string (position x y))
             xOrg
             yOrg))

  (define val     (board-ref x y))
  (when (or (man? val) (king? val))
    (define man-color (if (= WHITE (color val)) white-color black-color))
    (send dc set-brush man-color 'solid)
    (send dc set-smoothing 'aligned)
    (send dc draw-ellipse (+ xOrg (/ SCALE 8))
                          (+ yOrg (/ SCALE 8))
                          (* SCALE 6/8) 
                          (* SCALE 6/8)))
  (when (king? val) 
    (define crown (new dc-path%))
    (send crown move-to (+ xOrg (/ SCALE 4))  (+ yOrg (/ SCALE 4)))
    (send crown line-to (+ xOrg (/ SCALE 4))  (+ yOrg (/ SCALE 2)))
    (send crown line-to (+ xOrg (* SCALE 3/4))(+ yOrg (/ SCALE 2)))
    (send crown line-to (+ xOrg (* SCALE 6/8))(+ yOrg (/ SCALE 4)))
    (send crown line-to (+ xOrg (* SCALE 5/8))(+ yOrg (* SCALE 3/8)))
    (send crown line-to (+ xOrg (* SCALE 4/8))(+ yOrg (/ SCALE 4)))
    (send crown line-to (+ xOrg (* SCALE 3/8))(+ yOrg (* SCALE 3/8)))
    (send crown line-to (+ xOrg (/ SCALE 4))  (+ yOrg (/ SCALE 4)))
    (send dc draw-path crown)
))
           

;; teste si une dame peut prendre
(define (king-can-jump? pos)
  (define col (color (get-piece pos)))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (or (and (adverse? (+ x 1) (+ y 1)) (no-piece? (+ x 2) (+ y 2)))
      (and (adverse? (- x 1) (+ y 1)) (no-piece? (- x 2) (+ y 2)))
      (and (adverse? (+ x 1) (- y 1)) (no-piece? (+ x 2) (- y 2)))
      (and (adverse? (- x 1) (- y 1)) (no-piece? (- x 2) (- y 2)))))

; teste si un pion peut prendre
(define (man-can-jump? pos)
  (define col (color (get-piece pos)))
  (define forward (if (= col BLACK) +1 -1))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (or (and (adverse? (+ x 1) (+ y forward)) (no-piece? (+ x 2) (+ y forward forward)))
      (and (adverse? (- x 1) (+ y forward)) (no-piece? (- x 2) (+ y forward forward)))))

;; teste si un pion (ou une dame) peut prendre
(define (can-jump? pos)
  (define piece (get-piece pos))
  (cond [(king? piece) (king-can-jump? pos)]
        [else          (man-can-jump? pos)]))

;; teste si un pion ou une dame quelconque peut prendre
(define (someone-can-jump? color)
  (for/or ([pos (inclusive-range 1 32)])
    (and (same-color? color (get-piece pos))
         (can-jump? pos))))

;; déplace un pion
(define (move-piece! color from to-positions)
  (define forward-direction (if (= color BLACK) +1 -1))
  (define to                (first to-positions))
  (define piece             (get-piece from))
  (define is-king?          (king? piece))
  (define deltaX            (- (coordX to) (coordX from)))
  (define deltaY            (- (coordY to) (coordY from)))
  (when (not (and (empty? (rest to-positions))
                  (same-color? piece color)
                  (= EMPTY (get-piece to))
                  (= 1 (abs deltaX))
                  (= 1 (abs deltaY))
                  (or is-king?
                      (= deltaY forward-direction))))
    (displayln "Error, invalid move")
    (exit))
  (sleep 0.5)
  (set-piece! from EMPTY)
  (set-piece! to piece))

;; prend une pièce
(define (jump-piece! color from to-positions)
  (define to                (first to-positions))
  (define piece             (get-piece from))
  (when (not (jump-is-valid? color from to))
    (displayln "Error, invalid move")
    (exit))
  (sleep 0.25)
  (set-piece! to piece)
  (set-piece! from EMPTY)
  (sleep 0.25) ; petite animation, montre d'abord le saut
  (set-piece! (pos-between from to) EMPTY) ; puis le pion enlevé
  (cond [(empty? (rest to-positions))
         (when (can-jump? to)
           (displayln "Invalid move : incomplete jump")
           (exit))]
        [else (jump-piece! color to (rest to-positions))]))

(define (jump-is-valid? color from to)
  (define forward-direction (if (= color BLACK) +1 -1))
  (define x                 (coordX from))
  (define y                 (coordY from))
  (define piece             (get-piece from))
  (define is-king?          (king? piece))
  (define deltaX            (- (coordX to) x))
  (define deltaY            (- (coordY to) y))
  (and (same-color? piece color)
       (= EMPTY (get-piece to))
       (= 2 (abs deltaX))
       (= 2 (abs deltaY))
       (other-color? color (get-piece (pos-between from to)))
       (or (king? piece)
           (= forward-direction (sgn deltaY)))))


;; joue un coup complet
(define (play-move color move)
  (define jump? (equal? "x" (first move)))
  (when (and (not jump?) (someone-can-jump? color))
    (displayln "Invalid move : jump is mandatory")
    (exit))
  (define from (second move))
  (define dests (rest (rest move)))
  (cond [jump? (jump-piece! color from dests)]
        [else  (move-piece! color from dests)])
  (define final-pos (last move))
  (define piece     (get-piece final-pos))
  (when (or (and (= WHITE color) (< final-pos 5))
            (and (= BLACK color) (> final-pos 28)))
    (set-piece! final-pos (promote-king piece)))
  (sleep 1))

;; teste si la partie est finie (un joueur n'a plus de pions)
(define (game-over? board)
  (or (null? (filter white? (inclusive-range 1 32)))
      (null? (filter black? (inclusive-range 1 32)))))

(define (read-move-from-port port)
  (define move (read-line port))
  (display "Answer: ") (displayln move)
  (define separator (if (string-contains? move "x") "x" "-"))
  (define splitted  (string-split move separator))
  (cons separator (map string->number splitted)))

;; récupération des noms des programmes à exécuter et lancement des deux programmes
(define names (command-line #:args (name1 name2)
                            (list (if (equal? name1 "-") "You" name1)
                                  (if (equal? name2 "-") "You" name2))))

(define-values (process1 out1 in1 err1)
  (if (equal? (first names) "You")
      (values #f (current-input-port) (current-output-port) #f)
      (subprocess #f #f #f (first names) "1")))
(define-values (process2 out2 in2 err2)
  (if (equal? (second names) "You")
      (values #f (current-input-port) (current-output-port) #f)
      (subprocess #f #f #f (second names) "2")))

;; lecture du coup d'un joueur
(define (read-move color board)
  (define player-name (if (= color BLACK) (first names) (second names)))
  (display "Waiting for ") (displayln player-name)
  (define input-port   (if (= color BLACK) out1 out2))
  (cond [(equal? player-name "You") (send mycanvas get-move color)]
        [else (define move (read-move-from-port input-port))
              (play-move color move)
              move]))

;; envoi d'un coup à un joueur
(define (send-move-to color move)
  (define separator (first move))
  (define positions (rest move))
  (define output-port (if (= color BLACK) in1 in2))
  (define (send pos) (display separator output-port) (display pos output-port))
  (display (first positions) output-port)
  (map send (rest positions))
  (newline output-port)
  (flush-output output-port))

;; déroulement de la partie : fait jouer les deux joueurs
(define (run-match)
  (define (game-loop color turn-number)
    (display "\e[39m")
    (display "Move #") (displayln turn-number)
    (define player (if (= color BLACK) (first names) (second names)))
    (define move (read-move color board))
    (define opponent (opponent-color color))
    (send-move-to opponent move)
    (cond [(game-over? board) (display player) (displayln " wins!")]
          [else  (game-loop opponent (add1 turn-number))]))
;; placement initial des pions
  (for ([pos (inclusive-range  1 12)])(set-piece! pos BLACK))
  (for ([pos (inclusive-range 13 20)])(set-piece! pos EMPTY))
  (for ([pos (inclusive-range 21 32)])(set-piece! pos WHITE))
  (sleep 2)
  (game-loop BLACK 1))

;; Fabrique une procédure (fermeture) qui relaye un port vers le terminal,
;; en affichant une séquence de caractère avant chaque message

(define (relay-error-msgs port color-escape-sequence)
  (define msg (read-line port))
    (when (not (eof-object? msg))
      (display color-escape-sequence)
      (displayln msg)
      (flush-output)
      (relay-error-msgs port color-escape-sequence)))

(define (relay-thunk error-port msg-prologue)
  (lambda() (relay-error-msgs error-port msg-prologue)))


;; fenêtre de l'interface graphique
(define frame (new frame% [label (string-append (first names) " vs "  (second names))]))

(define my-canvas%
  (class canvas%
    (define-values (from to) (values #f #f))

    (define (wait-move color)
      (set! to false)
      (sleep 0.25)
      (unless (and to (same-color? color (get-piece from)))
        (wait-move color)))

    (define (do-jump color path)
      (define (get-next-jump color path)
        (wait-move color)
        (if (and (= from (first path))
                 (jump-is-valid? color from to))
            (do-jump color (cons to path))
            (get-next-jump color path)))
      (define move-to                (first path))
      (define move-from              (second path))
      (define piece                  (get-piece move-from))
      (sleep 0.25)
      (set-piece! move-to piece)
      (set-piece! move-from EMPTY)
      (sleep 0.25) ; petite animation, montre d'abord le saut
      (set-piece! (pos-between move-from move-to) EMPTY) ; puis le pion enlevé
      (cond [(can-jump? move-to)  (displayln "Please continue your jump...")
                                  (get-next-jump color path)]
            [else                 (cons "x" (reverse path))]))


    (define (get-move color)
      (wait-move color)
      (define deltaX (- (coordX to) (coordX from)))
      (define deltaY (- (coordY to) (coordY from)))
      (define jump?  (< 1 (max (abs deltaX) (abs deltaY))))
      (define piece  (get-piece from))
      (define forward-direction (if (= color BLACK) +1 -1))
      (define move
        (cond [(someone-can-jump? color)
               (if (jump-is-valid? color from to)
                   (do-jump color (list to from))
                   (get-move color))]
              [(and (= 1 (abs deltaX))
                    (= 1 (abs deltaY))
                    (= EMPTY (get-piece to))
                    (or (king? piece)
                        (= deltaY forward-direction)))
               (set-piece! from EMPTY)
               (set-piece! to piece)
               (list "-" from to)]
              [else (get-move color)]))
      (define final-pos (last move))
      (when (or (and (= WHITE color) (< final-pos 5))
                (and (= BLACK color) (> final-pos 28)))
        (set-piece! final-pos (promote-king piece)))
      move)

    (define/override (on-event event)
      (define x (quotient (send event get-x) SCALE))
      (define y (quotient (send event get-y) SCALE))
      (when (send event button-down? 'left)
        (set! from (position x y)))
      (when (send event button-up? 'left)
        (set! to   (position x y))))

    (public get-move)
    (super-new)))

(define (draw-board canvas dc)
  (for ([pos (range 1 33)])
    (draw-square pos dc)))

(define mycanvas (new my-canvas%
                      [parent frame]
                      [min-width  (* SCALE 8)]
                      [min-height (* SCALE 8)]
                      [paint-callback draw-board]))

(define dc (send mycanvas get-dc))
(send frame show #t)

(sleep 2) ; inutile, juste par superstition...

;; un thread par joueur pour relayer sa sortie erreur vers le terminal
(when err1 (thread (relay-thunk err1 "\e[93m"))) ; affichage en jaune
(when err2 (thread (relay-thunk err2 "\e[96m"))) ; affichage en cyan

(define game (yield (thread run-match)))
