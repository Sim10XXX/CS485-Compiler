class Main inherits B{
    main() : SELF_TYPE {
	{
	    c();
	}
    };
};

class A{
    self() : SELF_TYPE{{
        self;
    }};
};

class B inherits A{
    a : A;
    b : B;

    c
    () : SELF_TYPE{{
        a<- (new B)@A.self();
        b
        <- 
        (new 
        A)@
        B.self();
        self;
    }};
};