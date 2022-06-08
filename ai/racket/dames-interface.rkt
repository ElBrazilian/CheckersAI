#lang racket
(require racket/gui)

;; définitions modifiables pour configurer l'affichage
(define SCALE 90)            ; taille en pixel des cases
(define pawn-size (* SCALE 2/3)); taille des pions dessinés
(define queen-size (/ SCALE 3)); taille des dames
(define black-color  "Red")   ; couleur des pions du joueur "noir"
(define white-color  "White") ; couleur des pions du joueur "blanc"
(define square-color "Green") ; couleur des cases utilisées
(define empty-color  "White") ; couleur des cases non utilisées

;; on représente les pions par des bits séparés
(define EMPTY                0)
(define BLACK                1)
(define WHITE                2)
(define QUEEN                4)
(define BLACK-QUEEN          (bitwise-ior BLACK QUEEN))
(define WHITE-QUEEN          (bitwise-ior WHITE QUEEN))
(define OUTSIDE              8)
(define COLOR-MASK           (bitwise-ior BLACK WHITE))
(define (color piece)        (bitwise-and piece COLOR-MASK))
(define (opponent-color a)   (bitwise-xor a COLOR-MASK))
(define (other-color? a b)   (= COLOR-MASK (bitwise-ior (color a) (color b))))
(define (same-color? a b)    (= (color a) (color b)))
(define (pawn? piece)        (not (= EMPTY (color piece))))
(define (queen? piece)       (not (zero? (bitwise-and piece QUEEN))))
(define (promote-queen piece) (bitwise-ior piece QUEEN))

;; quelques fonctions pour convertir la notation officielle Manoury
;; en coordonnées (x,y), et vice-versa
(define (position x y)       (+ 1 (quotient x 2) (* 5 y)))
(define (coordY pos)         (quotient (sub1 pos) 5))
(define (coordX pos)         (+ (* 2 (modulo (sub1 pos) 5))
                                1 (- (modulo (coordY pos) 2))))

;; quelques fonctions pour accéder au damier, représenté par un vecteur de vecteurs
(define board                (for/vector ([_ 10]) (make-vector 10 EMPTY)))
(define (board-ref x y)      (cond [(and (< -1 x 10) (< -1 y 10)) (vector-ref (vector-ref board y) x)]
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
  (define x     (coordX from))
  (define y     (coordY from))
  (define dx    (sgn (- (coordX to) x)))
  (define dy    (sgn (- (coordY to) y)))
  (define steps (+ 1 (empty-steps (+ x dx) (+ y dy) dx dy)))
  (position (+ x (* steps dx)) (+ y (* steps dy))))

(define (empty-steps x y dx dy)
  (if (no-piece? x y)
      (+ 1 (empty-steps (+ x dx) (+ y dy) dx dy))
      0))
(define (empty-between? x y deltaX deltaY)
  (define dx (sgn deltaX))
  (define dy (sgn deltaY))
  (define distance (abs deltaX))
  (for/and ([step (in-range 1 distance)])
           (no-piece? (+ x (* step dx))
                      (+ y (* step dy)))))

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
  (when (or (pawn? val) (queen? val))
    (define pawn-color (if (= WHITE (color val)) white-color black-color))
    (send dc set-brush pawn-color 'solid)
    (send dc set-smoothing 'aligned)
    (send dc draw-ellipse (+ xOrg (/ SCALE 8))
                          (+ yOrg (/ SCALE 8))
                          (* SCALE 6/8) 
                          (* SCALE 6/8)))
  (when (queen? val) 
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
(define (queen-can-take? pos)
  (define col (color (get-piece pos)))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (for*/or ([dx (list +1 -1)] [dy (list +1 -1)])
       (define steps (+ 1 (empty-steps (+ x dx) (+ y dy) dx dy)))
       (and (adverse?  (+ x (* steps dx)   ) (+ y (* steps dy)   ))
            (no-piece? (+ x (* steps dx) dx) (+ y (* steps dy) dy)))))

;; teste si une dame peut prendre encore
(define (queen-can-take-again? pos taken-list)
  (define col (color (get-piece pos)))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (for*/or ([dx (list +1 -1)] [dy (list +1 -1)])
       (define steps  (+ 1 (empty-steps (+ x dx) (+ y dy) dx dy)))
       (define taken-pos (position (+ x (* steps dx)) (+ y (* steps dy))))
       (and (adverse?  (+ x (* steps dx)   ) (+ y (* steps dy)   ))
            (not (member taken-pos taken-list))
            (no-piece? (+ x (* steps dx) dx) (+ y (* steps dy) dy)))))

; teste si un pion peut prendre
(define (pawn-can-take? pos)
  (define col (color (get-piece pos)))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (for*/or ([dx (list +1 -1)] [dy (list +1 -1)])
           (and (adverse?  (+ x dx)    (+ y dy))
                (no-piece? (+ x dx dx) (+ y dy dy)))))

; teste si un pion peut prendre encore
(define (pawn-can-take-again? pos taken-list)
  (define col (color (get-piece pos)))
  (define x (coordX pos))
  (define y (coordY pos))
  (define (adverse? x y) (other-color? col (board-ref x y)))
  (for*/or ([dx (list +1 -1)] [dy (list +1 -1)])
           (define pos (position (+ x dx) (+ y dy)))
           (and (adverse?  (+ x dx)    (+ y dy))
                (not (member pos taken-list))  ; on ne peut pas prendre 2 fois le même pion
                (no-piece? (+ x dx dx) (+ y dy dy)))))

;; teste si un pion (ou une dame) peut prendre
(define (can-take? pos)
  (define piece (get-piece pos))
  (cond [(queen? piece) (queen-can-take? pos)]
        [else           (pawn-can-take? pos)]))

(define (can-take-again? pos taken-list)
  (define piece (get-piece pos))
  (cond [(queen? piece) (queen-can-take-again? pos taken-list)]
        [else           (pawn-can-take-again? pos taken-list)]))

;; teste si un pion ou une dame quelconque peut prendre
(define (someone-can-take? color)
  (for/or ([pos (inclusive-range 1 50)])
    (and (same-color? color (get-piece pos))
         (can-take? pos))))

;; déplace un pion
;(define (move-piece! color from to-positions)
;  (define forward-direction (if (= color BLACK) +1 -1))
;  (define to                (first to-positions))
;  (define piece             (get-piece from))
;  (define is-queen?         (queen? piece))
;  (define x                 (coordX from))
;  (define y                 (coordY from))
;  (define deltaX            (- (coordX to) x))
;  (define deltaY            (- (coordY to) y))
;  (define dx                (/ deltaX (abs deltaX)))
;  (define dy                (/ deltaY (abs deltaY)))
;  (when (not (and (empty? (rest to-positions))
;                  (same-color? piece color)
;                  (= EMPTY (get-piece to))
;                  (= (abs deltaX) (abs deltaY))
;                  (if is-queen?
;                      (>= (empty-steps (+ x dx) (+ y dy) dx dy) (abs deltaX))
;                      (and (= 1 (abs deltaX))
;                           (= deltaY forward-direction)))))
;    (displayln "Erreur, coup illégal")
;    (exit))
;  (sleep 0.5)
;  (set-piece! from EMPTY)
;  (set-piece! to piece)
;  (sleep 0.5))

;; prend une pièce
;(define (take-piece! color from to-positions [taken-list empty])
;  (define to                (first to-positions))
;  (define piece             (get-piece from))
;  (define taken-pos         (take-is-valid? color from to))
;  (when (not taken-pos)
;    (displayln "Erreur, coup illégal")
;    (exit))
;  (set-piece! to piece)
;  (set-piece! from EMPTY)
;  (sleep 0.5) ; petite animation, montre d'abord le saut
;  (when (member taken-pos taken-list)
;    (displayln "Coup illégal: prise de deux fois la même pièce")
;    (exit))
;  (define taken (cons taken-pos taken-list))
;  (cond [(empty? (rest to-positions))
;         (when (can-take-again? to taken)
;           (displayln "Coup illégal: la prise multiple est incomplète")
;           (exit))
;         (for ([pos taken])
;              (sleep 0.25) ; laisse le temps de voir
;              (set-piece! pos EMPTY)) ; supprime les prises
;        ]
;        [else (take-piece! color to (rest to-positions) taken)]))


; renvoie false si invalide, ou la position de la pièce prise si valide
(define (valid-queen-take? color x y deltaX deltaY)
  (define distanceX         (abs deltaX))
  (define distanceY         (abs deltaY))
  (define dx                (/ deltaX distanceX))
  (define dy                (/ deltaY distanceY))
  (define steps-before      (+ 1 (empty-steps (+ x dx) (+ y dy) dx dy)))
  (define steps-after       (empty-steps (+ x deltaX) (+ y deltaY) (- dx) (- dy)))
  (define between-x         (+ x (* steps-before dx)))
  (define between-y         (+ y (* steps-before dy)))
  (and (= distanceX (+ steps-before steps-after))    ; i.e une seule pièce sautée
       (other-color? color (get-piece (position between-x between-y)))
       (position between-x between-y))) 
                                               
; renvoie false si invalide, ou la position de la pièce prise si valide
(define (take-is-valid? color from to)
  (define x                 (coordX from))
  (define y                 (coordY from))
  (define piece             (get-piece from))
  (define is-queen?         (queen? piece))
  (define deltaX            (- (coordX to) x))
  (define deltaY            (- (coordY to) y))
  (and (same-color? piece color)
       (= EMPTY (get-piece to))
       (= (abs deltaX) (abs deltaY))
       (if is-queen?
           (valid-queen-take? color x y deltaX deltaY)
           (and (= 2 (abs deltaX))
                (= 2 (abs deltaY))
                (other-color? color (get-piece (pos-between from to)))
                (list (pos-between from to))))))


;; joue un coup complet
(define (play-move color move)
  (define take? (equal? "x" (first move)))
  (when (and (not take?) (someone-can-take? color))
    (displayln "Coup illégal: la prise est obligatoire")
    (exit))
  (define from      (second move))
  (define dest      (third  move))
  (define taken-pos (drop move 3))
  (define piece     (get-piece from))
  (set-piece! from EMPTY) (sleep 0.5) (set-piece! from piece) (sleep 0.5)  ; petite animation
  (set-piece! from EMPTY) 
  (set-piece! dest piece) (sleep 0.5) (set-piece! dest EMPTY) (sleep 0.5)  ; petite animation
  (set-piece! dest piece)
  (sleep 1)
  (for ([pos (reverse taken-pos)])
       (set-piece! pos EMPTY)
       (sleep 0.5))
  (when (or (and (= WHITE color) (< dest 6))
            (and (= BLACK color) (> dest 45)))
    (set-piece! dest (promote-queen piece))
    (sleep 0.5))
  (sleep 1))

;; teste si la partie est finie (un joueur n'a plus de pions)
(define (game-over?)
  (or (empty? (filter white? (inclusive-range 1 50)))
      (empty? (filter black? (inclusive-range 1 50)))))

;; lecture du coup d'un programme:
;; une ligne ??-??
;; ou deux lignes : ??x?? puis (liste des positions prises)
(define (read-move-from-port port)
  (define move       (read-line port))
  (display "Réponse : ")
  (displayln move)
  (define is-a-take? (string-contains? move "x"))
  (define separator  (if is-a-take? "x" "-"))
  (define splitted   (string-split move separator))
  (define taken      (if is-a-take? (read port) empty))
  (when is-a-take? (read-line port))
  (cons separator (append (map string->number splitted) taken)))

;; récupération des noms des programmes à exécuter et lancement des deux programmes
(define names (command-line #:args (name1 name2)
                            (list (if (equal? name1 "-") "Vous" name1)
                                  (if (equal? name2 "-") "Vous" name2))))

(define-values (process1 out1 in1 err1)
  (if (equal? (first names) "Vous")
      (values #f (current-input-port) (current-output-port) #f)
      (subprocess #f #f #f (first names) "1" "2000")))
(define-values (process2 out2 in2 err2)
  (if (equal? (second names) "Vous")
      (values #f (current-input-port) (current-output-port) #f)
      (subprocess #f #f #f (second names) "2" "2000")))

;; lecture du coup d'un joueur
(define (read-move color board)
  (define player-name (if (= color WHITE) (first names) (second names)))
  (display "En attente du joueur ") (displayln player-name)
  (define input-port   (if (= color WHITE) out1 out2))
  (cond [(equal? player-name "Vous") (send mycanvas get-move color)]
        [else (define move (read-move-from-port input-port))
              (play-move color move)
              move]))

;; envoi d'un coup à un joueur
(define (send-move-to color move)
  (define separator   (first  move))
  (define from        (second move))
  (define to          (third  move))
  (define taken-pos   (drop   move 3))
  (define output-port (if (= color WHITE) in1 in2))
  (display from output-port)
  (display separator output-port)
  (display to output-port)
  (newline output-port)
  (when (equal? separator "x")
    (display "Les prises : ") (displayln taken-pos)
    (display taken-pos output-port)
    (newline output-port))
  (flush-output output-port))

;; déroulement de la partie : fait jouer les deux joueurs
(define (run-match)
  (define (game-loop color turn-number)
    (display "\e[39m")
    (display "Coup #") (displayln turn-number)
    (define player (if (= color WHITE) (first names) (second names)))
    (define move (read-move color board))
    (define opponent (opponent-color color))
    (send-move-to opponent move)
    (cond [(game-over?) (display player) (displayln " gagne !")]
          [else  (game-loop opponent (add1 turn-number))]))
;; placement initial des pions
  (for ([pos (inclusive-range  1 20)])(set-piece! pos BLACK))
  (for ([pos (inclusive-range 21 30)])(set-piece! pos EMPTY))
  (for ([pos (inclusive-range 31 50)])(set-piece! pos WHITE))
  (sleep 2)
  (game-loop WHITE 1))

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

    (define (do-take color path [taken-list empty])
      (define (get-next-take color path taken-list)
        (wait-move color)
        (if (and (= from (first path))
                 (take-is-valid? color from to))
            (do-take color (cons to path) taken-list)
            (get-next-take color path taken-list))) ; recommence la saisie si la prise est invalide
      (define move-to                (first path))
      (define move-from              (second path))
      (define piece                  (get-piece move-from))
      (sleep 0.25)
      (set-piece! move-to piece)
      (set-piece! move-from EMPTY)
      (define taken (pos-between move-from move-to))
      (cond [(can-take-again? move-to (cons taken taken-list))
                                  (displayln "Continuez votre prise multiple...")
                                  (get-next-take color path (cons taken taken-list))]

            [else                 (set-piece! taken EMPTY) ; supprime le dernier pion pris
                                  (for ([taken taken-list]) (set-piece! taken EMPTY)) ; et tous les autres
                                  (define dest (first path))
                                  (when (or (and (= WHITE color) (< dest 6))
                                            (and (= BLACK color) (> dest 45)))
                                    (set-piece! dest (promote-queen piece)))
                                  (append (list "x" (last path) dest taken) taken-list)]))


    (define (get-move color)
      (wait-move color)
      (define deltaX (- (coordX to) (coordX from)))
      (define deltaY (- (coordY to) (coordY from)))
      (define piece  (get-piece from))
      (define forward-direction (if (= color BLACK) +1 -1))
      (define move
        (cond [(someone-can-take? color)
               (cond [(take-is-valid? color from to) (do-take color (list to from))]
                     [else   (displayln "La prise est obligatoire...")
                             (get-move color)])]       ; recommence la saisie si la prise est invalide...
              [(or (and (= 1 (abs deltaX))
                        (= 1 (abs deltaY))
                        (= EMPTY (get-piece to))
                        (or (queen? piece)
                            (= deltaY forward-direction)))
                   (and (queen? piece)
                        (= (abs deltaX) (abs deltaY))
                        (= EMPTY (get-piece to))
                        (empty-between? (coordX from) (coordY from) deltaX deltaY)))
               (set-piece! from EMPTY)
               (set-piece! to piece)
               (when (or (and (= WHITE color) (< to 6))
                         (and (= BLACK color) (> to 45)))
                  (set-piece! to (promote-queen piece)))
               (list "-" from to)]
              [else (get-move color)]))
      (sleep 1)
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
  (for ([pos (range 1 51)])
    (draw-square pos dc)))

(define mycanvas (new my-canvas%
                      [parent frame]
                      [min-width  (* SCALE 10)]
                      [min-height (* SCALE 10)]
                      [paint-callback draw-board]))

(define dc (send mycanvas get-dc))
(send frame show #t)

(sleep 2) ; inutile, juste par superstition...

; un thread par joueur pour relayer sa sortie erreur vers le terminal
(when err1 (thread (relay-thunk err1 "\e[93m"))) ; affichage en jaune
(when err2 (thread (relay-thunk err2 "\e[96m"))) ; affichage en cyan

(define game (yield (thread run-match)))
