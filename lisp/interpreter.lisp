

(defmacro ! (proc args)
  (apply proc args))

(defmacro define (sym form)
  `(let ((val (eval ,form)))
	 (progn (if (typep val 'function) (defun ,sym (&rest fargs) (apply val fargs)))
			(defparameter ,sym val))))

