class Main inherits IO{
    a : A;
    main() : Object{{
        let x : Int in{
            x <- 24;
            let x : Int in{
                x <- 9000;
                out_int(x/25);
                out_int(~x);
            };
            out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            a<- new A;
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);out_int(x+5);
            out_int(x-1000);
            out_int(x+x+x+x+x+x+x);
            out_int(x*50);
            out_int(x/2);
            if 0 <= x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 < x
            then out_int(1)
            else out_int(0)
            fi;
            if 0 = x
            then out_int(1)
            else out_int(0)
            fi;
            if false = (x < 1000)
            then out_int(1)
            else out_int(0)
            fi;
            out_int(a.get());
            out_int(a.set(2));

            out_int(a.set(2+a.get()));
            out_int(a.set(2+a.get()));
            out_int(a.set(2+a.get()));
            out_int(a.set(2+a.get()));
            out_int(a.set(2+a.get()));
            out_int(a.set(2+a.get()));
            out_int(2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000);
            out_int(2000000000+2000000000+2000000000+2000000000);
            if not true
            then out_int(1)
            else out_int(0)
            fi;
            let x : Int in{
                x <- 3;
                let y : Int in{
                    y <- x;
                    out_int(x);
                    out_int(y);
                    y <- 5;
                    out_int(x);
                    out_int(y);
                };
            };
            out_int(x);
        };
    }};
};

Class A{
    a : Int;
    get(): Int{a};
    set (x : Int) : Int{
        a <-x
    };
};