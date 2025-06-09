class Main{
    main() : SELF_TYPE {
	{
	    self;
	}
    };
};

class X inherits IO{
    x : Int;
    x(x : String) : Int{
        {
            out_string(x);
            let x : Int <- 5, x : Int <- x, x : Int <- x("b") in {
                out_int(x);
            };
            out_string(x);
            out_string("\n");
            6;
        }
    };
};