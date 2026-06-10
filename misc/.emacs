;;turn off startup message, its annoying
(setq inhibit-startup-message t)
;;turn the hell off this annoying noise
(setq ring-bell-function 'ignore)
;;bye bye toolbar
(tool-bar-mode -1)
;;bye bye scroll bar
(scroll-bar-mode -1)
;;bye bye tooltips
(tooltip-mode -1)
;;idk what this was
(set-fringe-mode 10)
;;turn off menu
(menu-bar-mode -1)
(global-display-fill-column-indicator-mode -1)
;;(require 'package)
;;(add-to-list 'package-archives '("melpa" . "https://melpa.org/packages/") t)
;;(package-initialize)


;;again, turn off autosaves
(setq auto-save-default nil)

;;turning on columns but it diddn't seem to work
(setq column-number-mode t)

;;turning on max font decor so we can actually differentiate variables
(setq font-lock-maximum-decoration t)

;;Turn the fuck off auto-save who wants that garbage when you can just learn how to save your files
(custom-set-variables
 ;; custom-set-variables was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 '(custom-enabled-themes '(wheatgrass))
 '(custom-safe-themes
   '("8ba8918be4bb12c57cae812f8f9543e7a4b59a3cc1d5d4a4f97dc26a397c94e3" "95b0bc7b8687101335ebbf770828b641f2befdcf6d3c192243a251ce72ab1692" default))
 '(package-selected-packages '(monokai-theme jazz-theme)))
(custom-set-faces
 ;; custom-set-faces was added by Custom.
 ;; If you edit it by hand, you could mess it up, so be careful.
 ;; Your init file should contain only one such instance.
 ;; If there is more than one, they won't work right.
 )

;;normal person doing normal indents (no java brackets!)
(setq c-default-style "linux"
      c-basic-offset 4)

(c-add-style "sane-human-style"
	     '("linux"
	       (brace-list-open . 0)
	       (brace-list-close .4)
	       (substatement-open . 4)))

(setq c-default-style "sane-human-style")

(add-hook 'c-mode-common-hook
	  (lambda()
	    (local-set-key (kbd "C-c o") 'ff-find-other-file)))


;;jazz thememe

(load-file "W:/misc/elpa/jazz-theme-20230814.1916/jazz-theme.el")
(load-theme 'jazz t)



;;setting emacs to fullscreen on creation
(add-to-list 'default-frame-alist '(fullscreen . maximized))


;;changing the size of the font
(set-face-attribute 'default nil :height 108)


;;Saving our desktops when emacs reopens

(setq desktop-dirname "s:/misc/EmacsDesktops"
      desktop-base-file-name "emacs.desktop"
      desktop-path (list desktop-dirname)
      desktop-save t
      desktop-load-locked-desktop nil
      desktop-auto-save-timeout 30)

(desktop-save-mode t)
(desktop-read)

(setq make-backup-files nil)



;;Setup for running build with key press, will need to come back here whenever you want to build another
;;file 
(require 'comint)

(defun compile-code ()
  "Compiles code in buffer"
  (interactive)
  (let ((buffer (get-buffer-create "*Compiler Shell*"))
	(set-done nil))
    (with-current-buffer buffer

      (unless (get-buffer-process buffer)
	(shell buffer))
      
      (unless (local-variable-p 'setup-done)
	(setq-local setup-done t)
	(comint-send-string buffer "s:\\misc\\shell.bat\n"))
      
      (comint-send-string buffer "s:\\code\\build.bat\n"))
    (switch-to-buffer buffer)))

(global-set-key (kbd "M-m") 'compile-code)

;;Search and replace bc the setup one is terrible
(global-set-key (kbd "C-c r") 'query-replace)
(global-set-key (kbd "C-c a") 'query-replace-regexp)
					; Bright-red TODOs

 (make-face 'font-lock-fixme-face)
 (make-face 'font-lock-note-face)
 (mapc (lambda (mode)
	 (font-lock-add-keywords
	  mode
	  '(("\\<\\(TODO\\)" 1 'font-lock-fixme-face t)
            ("\\<\\(NOTE\\)" 1 'font-lock-note-face t))))
	fixme-modes)
 (modify-face 'font-lock-fixme-face "Red" nil nil t nil t nil nil)
 (modify-face 'font-lock-note-face "Dark Green" nil nil t nil t nil nil)


;;TODO


;; ;; Define a custom face for highlighting TODO
;; (defface my-todo-face
;;   '((t (:foreground "red" :weight bold)))
;;   "Face for highlighting TODO keywords.")

;; (defface my-note-face
;;   '((t (:foreground "green" :weight bold)))
;;   "Face for highlighting NOTE keywords.")

;; ;; Function to add custom keyword highlighting (e.g., TODO)
;; (defun highlight-custom-keywords ()
;;   "Highlight custom keywords like TODO: and NOTE:."
;;   (font-lock-add-keywords
;;    nil
;;    '(("\\<\\(TODO:\\)" 1 'my-todo-face t)
;;      ("\\<\\(NOTE:\\)" 1 'my-note-face t)))
;;   (font-lock-flush)
;;   (font-lock-ensure))

;; Apply the highlighting in both text and programming modes
;;(add-hook 'text-mode-hook 'highlight-custom-keywords)
;;(add-hook 'prog-mode-hook 'highlight-custom-keywords)



