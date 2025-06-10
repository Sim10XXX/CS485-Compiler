Class Main inherits IO {
    x : B <- new B;
    y : A <- new B;
    main() : Object{{
        out_int(x@A.g());
        out_string("\n");
        out_int(x@B.g());
        out_string("\n");
        out_int(y@A.g());
        out_string("\n");
    }};
        
};

Class A{
    a : Int;
    g() : Int{
        a
    };
};

class B inherits A{
    b : Int <- 5;
    g() : Int{
        100
    };
};