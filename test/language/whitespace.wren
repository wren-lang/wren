// No indent
System.print("ok") // expect: ok

// Indent with space
  System.print("ok") // expect: ok

// Indent with tab
	System.print("ok") // expect: ok

// Indent with space then tab
  	System.print("ok") // expect: ok

// Indent with tab then space
	  System.print("ok") // expect: ok

// Indent with mixed tab and space
  	  	System.print("ok") // expect: ok

// Space in with code
System . print ( "ok" ) // expect: ok

// Tab in with code
System	.	print	(	"ok"	) // expect: ok

// Tab and space mixed in with code
System	 . 	print 	(	 "ok" 	) // expect: ok
