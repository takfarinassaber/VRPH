(macrolet ((check-bounds (sequence start end)
             (let ((length (gensym (string '#:length))))
               `(let ((,length (length ,sequence)))
                  (check-type ,start unsigned-byte "a non-negative integer")
                  (when ,end (check-type ,end unsigned-byte "a non-negative integer or NIL"))
                  (unless ,end
                    (setf ,end ,length))
                  (unless (<= ,start ,end ,length)
                    (error "Wrong sequence bounds. start: ~S end: ~S" ,start ,end))))))

  (defun split-sequence (delimiter sequence &key (start 0) (end nil) (from-end nil)
                         (count nil) (remove-empty-subseqs nil)
                         (test #'eql) (test-not nil) (key #'identity))
    "Return a list of subsequences in seq delimited by delimiter.

If :remove-empty-subseqs is NIL, empty subsequences will be included
in the result; otherwise they will be discarded.  All other keywords
work analogously to those for CL:SUBSTITUTE.  In particular, the
behaviour of :from-end is possibly different from other versions of
this function; :from-end values of NIL and T are equivalent unless
:count is supplied. The second return value is an index suitable as an
argument to CL:SUBSEQ into the sequence indicating where processing
stopped."
    (check-bounds sequence start end)
    (cond
      ((and (not from-end) (null test-not))
       (split-from-start (lambda (sequence start)
                           (position delimiter sequence :start start :key key :test test))
                         sequence start end count remove-empty-subseqs))
      ((and (not from-end) test-not)
       (split-from-start (lambda (sequence start)
                           (position delimiter sequence :start start :key key :test-not test-not))
                         sequence start end count remove-empty-subseqs))
      ((and from-end (null test-not))
       (split-from-end (lambda (sequence end)
                         (position delimiter sequence :end end :from-end t :key key :test test))
                       sequence start end count remove-empty-subseqs))
      ((and from-end test-not)
       (split-from-end (lambda (sequence end)
                         (position delimiter sequence :end end :from-end t :key key :test-not test-not))
                       sequence start end count remove-empty-subseqs))))

  (defun split-sequence-if (predicate sequence &key (start 0) (end nil) (from-end nil)
                            (count nil) (remove-empty-subseqs nil) (key #'identity))
    "Return a list of subsequences in seq delimited by items satisfying
predicate.

If :remove-empty-subseqs is NIL, empty subsequences will be included
in the result; otherwise they will be discarded.  All other keywords
work analogously to those for CL:SUBSTITUTE-IF.  In particular, the
behaviour of :from-end is possibly different from other versions of
this function; :from-end values of NIL and T are equivalent unless
:count is supplied. The second return value is an index suitable as an
argument to CL:SUBSEQ into the sequence indicating where processing
stopped."
    (check-bounds sequence start end)
    (if from-end
        (split-from-end (lambda (sequence end)
                          (position-if predicate sequence :end end :from-end t :key key))
                        sequence start end count remove-empty-subseqs)
        (split-from-start (lambda (sequence start)
                            (position-if predicate sequence :start start :key key))
                          sequence start end count remove-empty-subseqs)))

  (defun split-sequence-if-not (predicate sequence &key (count nil) (remove-empty-subseqs nil)
                                (from-end nil) (start 0) (end nil) (key #'identity))
    "Return a list of subsequences in seq delimited by items satisfying
\(CL:COMPLEMENT predicate).

If :remove-empty-subseqs is NIL, empty subsequences will be included
in the result; otherwise they will be discarded.  All other keywords
work analogously to those for CL:SUBSTITUTE-IF-NOT.  In particular,
the behaviour of :from-end is possibly different from other versions
of this function; :from-end values of NIL and T are equivalent unless
:count is supplied. The second return value is an index suitable as an
argument to CL:SUBSEQ into the sequence indicating where processing
stopped."
    (check-bounds sequence start end)
    (if from-end
        (split-from-end (lambda (sequence end)
                          (position-if-not predicate sequence :end end :from-end t :key key))
                        sequence start end count remove-empty-subseqs)
        (split-from-start (lambda (sequence start)
                            (position-if-not predicate sequence :start start :key key))
                          sequence start end count remove-empty-subseqs))))

(defun split-from-end (position-fn sequence start end count remove-empty-subseqs)
  (loop
     :for right := end :then left
     :for left := (max (or (funcall position-fn sequence right) -1)
                       (1- start))
     :unless (and (= right (1+ left))
                  remove-empty-subseqs) ; empty subseq we don't want
     :if (and count (>= nr-elts count))
     ;; We can't take any more. Return now.
       :return (values (nreverse subseqs) right)
     :else
       :collect (subseq sequence (1+ left) right) into subseqs
       :and :sum 1 :into nr-elts
     :until (< left start)
   :finally (return (values (nreverse subseqs) (1+ left)))))

(defun split-from-start (position-fn sequence start end count remove-empty-subseqs)
  (let ((length (length sequence)))
    (loop
       :for left := start :then (+ right 1)
       :for right := (min (or (funcall position-fn sequence left) length)
                          end)
       :unless (and (= right left)
                    remove-empty-subseqs) ; empty subseq we don't want
       :if (and count (>= nr-elts count))
       ;; We can't take any more. Return now.
         :return (values subseqs left)
       :else
         :collect (subseq sequence left right) :into subseqs
         :and :sum 1 :into nr-elts
       :until (>= right end)
     :finally (return (values subseqs right)))))

(pushnew :split-sequence *features*)

(defun getenv (name)
  (declare (simple-string name))
  #+abcl (system::getenv name)
  #+allegro (sys:getenv name)
  #+cmu (cdr (assoc name ext:*environment-list* :test #'string-equal))
  #+ccl (ccl:getenv name)
  #+clisp (ext:getenv name)
  #+(or ecl gcl mkcl) (si:getenv name)
  #+lispworks (lispworks:environment-variable name)
  #+sbcl (sb-ext:posix-getenv name))

(defun split-at (item sequence &key (start 0))
  (let ((len (length sequence)))
    (labels ((split-from (start)
	       (unless (>= start len)
		 (let ((sep (position item sequence :start start)))
		   (cond ((not sep)
			  (list (subseq sequence start)))
			 ((> sep start)
			  (cons (subseq sequence start sep)
				(split-from (1+ sep))))
			 (:else
			  (split-from (1+ start))))))))
      (split-from start))))

(let* ((bin-path (mapcar #'(lambda (string)
				(pathname (concatenate 'string 
						       string 
						       #+win32 "\\" #-win32 "/")))
		      (split-at #-win32 #\: #+win32 #\;
				(or (getenv "PATH")
				    #+win32 "C:\\WINDOWS\\System32"
				    #-win32 "/bin:/usr/bin:/usr/local/bin")))))
  (defun find-executable (name)
    (loop
       for dir in bin-path
       for pathname = (make-pathname :defaults dir :name name)
       when (probe-file pathname)
       return (namestring (truename pathname)))))


(defun nodes (filename)
  (declare (type simple-string filename))
  (with-open-file (stream filename :direction :input)
    (let (line
	  dimension
	  depot-cluster
	  nodes)
      (tagbody 
       :top
	 (setq line (read-line stream))
	 (when (string/= (subseq line 0 9) "DIMENSION")
	   (go :top)))
      (setq dimension (parse-integer (car (last (split-sequence  #\Space line)))))
      (tagbody 
       :top
	 (setq line (read-line stream))
	 (when (or (< (length line) 18)
		   (string/= (subseq line 0 18) "NODE_COORD_SECTION"))
	   (go :top)))

      
      (dotimes (i dimension)
	(let ((a (read stream))
	      (b (read stream))
	      (c (read stream)))

	  (setf nodes (append nodes (list(list a b c))))))
      
      
      (tagbody 
       :top
	 (setq line (read-line stream))
	 (when (or (< (length line) 15)
		   (string/= (subseq line 0 15) "CLUSTER_SECTION"))
	   (go :top)))

      (dotimes (i dimension)
	(let ((id (read stream))
	      (cluster (read stream)))
	  (declare (ignore id))
	  (when (= i 0)
	    (setf depot-cluster cluster))
	  (setf (nth i nodes) (append (nth i nodes) (list cluster)))))
      (values nodes depot-cluster))))


(defun generate-plot-file (filename nodes depot-cluster)
  (declare (type simple-string filename))
  (let* (result
	 worst
	 tmp
	 (basename (reverse (subseq (reverse filename) 6))))
    (format t "nodes:~%~A~%" nodes)
    ;; ugly
    (dolist (i nodes)
      (setq tmp (concatenate 'string basename "_data_" (write-to-string  (car (last i))) ".txt"))
      (handler-case (delete-file tmp) (error nil nil)))
    
    (dolist (i nodes)
      (setq tmp (concatenate 'string basename "_data_" (write-to-string  (car (last i))) ".txt"))
      (with-open-file (stream 
		       tmp 
		       :direction :output 
		       :if-exists :append 
		       :if-does-not-exist :create)
	(format stream "~{~A ~}~%" i)) ;; debug

      (let ((cluster (car (last i)))
	    (x (second i))
	    (y (third i)))
	(if (null (assoc cluster worst))
	    (pushnew (list cluster x y x y) worst)
	    (block nothing
	      (let* ((cluster-info (assoc cluster worst))
		     (cluster-x-worst (second cluster-info))
		     (cluster-y-worst (third cluster-info))
		     (cluster-x-best (fourth cluster-info))
		     (cluster-y-best (fifth cluster-info)))
		(when (< x  cluster-x-worst)
		  (setf (second (assoc cluster worst)) x))
		(when (> x cluster-x-best)
		  (setf (fourth (assoc cluster worst)) x))
		(when (< y  cluster-y-worst)
		  (setf (third (assoc cluster worst)) y))
		(when (> y cluster-y-best)
		  (setf (fifth (assoc cluster worst)) y)))))))
    (setf worst (sort worst #'(lambda (x y) (< (car x) (car y)))))
    (format t "worst:~%~A~%" worst) ;; debug
    (setq tmp (concatenate 'string basename "_cmd.txt"))
    (pushnew tmp result) ;; result construction
    (handler-case (delete-file tmp) (error nil nil))
    (format t "gnuplot commands :~%")
    (with-open-file (stream 
		     tmp 
		     :direction :output 
		     :if-exists :append 
		     :if-does-not-exist :create)
      (format stream "set terminal jpeg~%")
      (format t "set terminal jpeg~%")
      (format stream "set output \"~A.jpeg\"~%" filename)
      (format t "set output \"~A.jpeg\"~%" filename)
      (format stream "unset tics~%unset border~%set key rmargin~%")
      (format t "unset tics~%unset border~%set key rmargin~%")
      (dolist (i worst)
	(let ((two (- (second i) 0))  ;; 0.5
	      (three (- (third i) 0)) ;; 0.5
	      (four (+ (fourth i) 0)) ;; 0.5
	      (five (+ (fifth i) 0))) ;; 0.5
	(format stream 
		"set object ~A rect from ~A,~A to ~A,~A fc rgb 'white'~%" 
		(1+ (first i)) two three four five)
	(format t 
		"set object ~A rect from ~A,~A to ~A,~A fc rgb 'white'~%" 
		(1+ (first i)) two three four five)))
      (format stream "plot ")
      (format t "plot ")
      (dolist (i (subseq worst 0 (1- (length worst))))
	(setq tmp (concatenate 'string basename "_data_" (write-to-string  (car i)) ".txt"))
	(pushnew tmp result) ;; result construction
	(if (= (first i) depot-cluster)
	    (progn 
	      (format stream "'~A' using 2:3 lt ~A title 'depot', " tmp (first i))
	      (format t "'~A' using 2:3 lt ~A title 'depot', " tmp (first i)))
	    (progn 
	      (format stream "'~A' using 2:3 lt ~A title 'cluster ~A', " tmp (first i) (first i))
	      (format t "'~A' using 2:3 lt ~A title 'cluster ~A', " tmp (first i) (first i)))))
      (setq tmp (concatenate 'string 
			     basename 
			     "_data_" 
			     (write-to-string  (car (car (last worst)))) ".txt"))
      (pushnew tmp result) ;; result construction
      (if (=  (first (car (last worst))) depot-cluster)
	  (progn 
	    (format stream
		    "'~A' using 2:3 lt ~A title 'depot'" 
		    tmp (first (car (last worst))))
	    (format t
		    "'~A' using 2:3 lt ~A title 'depot'~%" 
		    tmp (first (car (last worst)))))

	  (progn 
	    (format stream
		    "'~A' using 2:3 lt ~A title 'cluster ~A'" 
		    tmp (first (car (last worst))) (first (car (last worst))))
	    (format t
		    "'~A' using 2:3 lt ~A title 'cluster ~A'~%" 
		    tmp (first (car (last worst))) (first (car (last worst)))))))
    result))


(defun plot (filename)
  (multiple-value-bind (nodes depot-cluster)
      (nodes filename)
    
  (let ((files (generate-plot-file filename nodes depot-cluster)))
    (sb-ext:run-program (find-executable "gnuplot") (list (car (last files))))
    (format t "files:~%~A~%" files)
    (mapcar #'(lambda (x) 
		(handler-case (delete-file x) (error nil nil)))
	    files)
    (format t "cluster-depot-number:~%~A~%" depot-cluster)
    nil)))

(defun print-help ()
  (format t "Usage : ~A <instance-ccvrp>~%" (first sb-ext:*posix-argv*)))

(defun main ()
  (if (= (length sb-ext:*posix-argv*) 2)
      (handler-case (plot (second sb-ext:*posix-argv*))
	(error nil (print-help)))
      (print-help)))

(save-lisp-and-die "clusters_plot" :toplevel #'main :executable t :compression t)

