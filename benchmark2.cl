class Main inherits IO {
    a : Int <- 20000;
    i : Int <- 0;
    b : Int <- in_int();
    main(): Object { { 
        while i < 200 loop{
            a <- a+a+b;
            i <- i + 1;
        }pool;
        out_int(a);
        } };
};
