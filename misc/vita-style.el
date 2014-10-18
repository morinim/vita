;;; vita-style.el --- Vita's C/C++/Python style

;; Copyright (C) 2014 EOS di Manlio Morini.
;;
;; This Source Code Form is subject to the terms of the Mozilla Public
;; License, v. 2.0. If a copy of the MPL was not distributed with this file,
;; You can obtain one at http://mozilla.org/MPL/2.0/

;; C++
(add-to-list 'auto-mode-alist '("\\.h\\'" . c++-mode))
(add-to-list 'auto-mode-alist '("\\.tcc\\'" . c++-mode))

(c-add-style "vita-style"
  '("bsd"
   (c-basic-offset . 2)                        ; indent by two spaces
   (indent-tabs-mode . nil)                    ; use spaces rather than tabs
   (c-offsets-alist . ((innamespace . [0]))))) ; suppress namespace indentation

(defun vita-c++-mode-hook ()
  (c-set-style "vita-style"))        ; use my-style defined above

(add-hook 'c++-mode-hook 'vita-c++-mode-hook)

(add-hook 'c++-mode-common-hook
                  (lambda()
                    (add-hook 'before-save-hook
                    'delete-trailing-whitespace nil t)))

;; Python
(setq-default tab-width 4)
(setq-default python-indent 4)
(add-hook 'python-mode-common-hook
                  (lambda()
                    (add-hook 'before-save-hook
                    'delete-trailing-whitespace nil t)))

;; Mix
(setq-default show-trailing-whitespace t)
(setq-default indicate-empty-lines t)

(add-hook 'before-save-hook 'copyright-update)

(show-paren-mode 1)
