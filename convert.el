;; This is handy for putting in games using xchess -i, then converting
;; them over to something the gnuchess book routines can parse.
(defun convert-xchess-to-gnuchess ()
  "Converts rest of buffer, containing xchess game listings, into gnuchess 
format."
  (interactive)
  (beginning-of-line)
  (while (not (eobp))
    (cond ((looking-at "[ 0-9][ 0-9]\\. ")
	   (delete-horizontal-space)
	   (forward-word 1)
	   (delete-char 1)		; period
	   (forward-word 1)		; after first move
	   (delete-char 12)		; ?, !, +, or !!!
	   (if (not (looking-at " [a-h][1-8][a-h][1-8]"))
	       (delete-horizontal-space)
	     (forward-word 1)
	     (delete-char 6)
	     (delete-horizontal-space))
	   (forward-line 1)
	   (beginning-of-line))
	  (t (kill-line 1)))))
