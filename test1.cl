Class Main inherits IO {
    a : A <- new A;
    b : B <- new B;
    i : Int <- 2;
    main() : Object{{
        out_int(case a of
            x : Object => 1;
            x : B => 2;
            x : A => 3;
        esac);
        out_int(case b of
            x : Object => 1;
            x : B => 2;
            x : A => 3;
        esac);
        out_int(case i of
            x : Object => 1;
            x : B => 2;
            x : A => 3;
        esac);
    }};
};

Class A {

};

Class B inherits A{

};