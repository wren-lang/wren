// No indent
IO.print("ok") // expect: ok

// Indent with space
  IO.print("ok") // expect: ok

// Indent with tab
	IO.print("ok") // expect: ok

// Indent with space then tab
  	IO.print("ok") // expect: ok

// Indent with tab then space
	  IO.print("ok") // expect: ok

// Indent with mixed tab and space
  	  	IO.print("ok") // expect: ok

// Space in with code
IO . print ( "ok" ) // expect: ok

// Tab in with code
IO	.	print	(	"ok"	) // expect: ok

// Tab and space mixed in with code
IO	 . 	print 	(	 "ok" 	) // expect: ok
