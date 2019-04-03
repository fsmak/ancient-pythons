;;; This file was automatically generated by the panel editor.
;;; If you read it into gnu emacs, it will automagically format itself.

(panel (prop help creator:user-panel-help)
(prop user-panel #t)
(label "Audio Control Panel")
(x 395)
(y 69)
(al (pnl_filled_vslider (name "outputgain")
(prop help creator:user-act-help)
(label "output gain")
(x 6.5)
(y 0.75)
(w 0.4)
(h 4.35)
(val 0.329)
(labeltype 13)
(downfunc move-then-resize)
)
(pnl_frame (prop help creator:user-frame-help)
(x 0.25)
(y 2.75)
(w 5.1)
(h 2.3)
(downfunc move-then-resize)
(al (pnl_filled_hslider (name "recordsize")
(prop help creator:user-act-help)
(label "recording length")
(x -0.75)
(w 3.3)
(h 0.4)
(val 0.1)
(labeltype 11)
(downfunc move-then-resize)
)
(pnl_label (prop help creator:user-act-help)
(label "(max 10 seconds)")
(x -0.75)
(y -0.75)
(downfunc move-then-resize)
)
(pnl_wide_button (name "recordbutton")
(prop help creator:user-act-help)
(label "record from microphone...")
(x -0.75)
(y 0.75)
(w 4.7)
(downfunc move-then-resize)
)
)
)
(pnl_wide_button (name "playbackbutton")
(prop help creator:user-act-help)
(label "playback to speaker")
(x 0.25)
(y 2)
(w 5.15)
(downfunc move-then-resize)
)
(pnl_wide_button (name "quitbutton")
(prop help creator:user-act-help)
(label "quit")
(x 0.25)
(y 0.25)
(w 1.75)
(downfunc move-then-resize)
)
)
)
;;; Local Variables:
;;; mode: scheme
;;; eval: (save-excursion (goto-char (point-min)) (kill-line 3))
;;; eval: (save-excursion (goto-char (point-min)) (replace-regexp "[ \n]*)" ")"))
;;; eval: (indent-region (point-min) (point-max) nil)
;;; eval: (progn (kill-line -3) (delete-backward-char 1) (save-buffer))
;;; End:
