class Main inherits A{
    main() : Main{
        case self of
            x : B => x;
            x : C => new Main;
            x : D => x;
        esac
    };
};

class A{
};
class B inherits A{
};
class C inherits B{
};
class D inherits A{
};