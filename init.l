(nick "Kanra")

(server
    (name "subluminal")
    (nick "kanra")
    (host "irc.sublumin.al" 6667)
    (nossl))


;; = Machine Grammar =
;; 
;; PROG := '' | WSPACE | SEXPR
;;       | SEXPR WSPACE PROG 
;; 
;; WSPACE := ' ' | ' ' WSPACE
;; 
;; SEXPR := ATOM | LIST
;; 
;; LIST := '(' EXPR ')'
;; 
;; ATOM := 
