 #Tea2Lang : A Iteration of a Teaching Programming Language for Compiler Theory and Practice

---
CPS2000 *Compiler Theory and Practice*

Assignment for the Academic year 2020/1

Xandru Mifsud (0173498M), B.Sc. (Hons) Mathematics and Computer Science

---

This repository defines the compilation pipeline for the _Tea2Lang_ language, an iteration of _TeaLang_, which adds additional
support for char types, the ```tlstruct``` struct type, 1-dimensional array types, and anonymous typing.

The _TeaLang_ family of languages are an expression-based strongly-typed family of programming languages. The languages 
have ```C```-style comments, that is, ```//. . .``` for line comments and ```/*. . . */``` for block comments. The languages
are case-sensitive and each function is expected to return a value. _Tea2Lang_ in particular has 5 primitive types: ‘float’,
‘int’, ‘bool’, ‘char’ and ‘string’. Binary operators, such as ‘+’, require that the operands have matching types;
the languages do not perform any implicit/automatic typecasting. The languages also support function overloading.

The following is a syntactically and semantically correct _Tea2Lang_ program:
```
// Function definition for Sum over an array of floats
auto Sum(x:float[], size:int){
    let y:auto = 0.0;

    for(let i:int = 0; i < size; i = i + 1){
        y = y + x[i];
    }

    return y;
}

// tlstruct defintion for a 3-dim vector
tlstruct Vector{
    let v[3]:float = {0.0};

    // sets the components of the 3dim vector
    int Set(v0:float, v1:float, v2:float){
        v[0] = v0;
        v[1] = v1;
        v[2] = v2;

        return 0;
    }

    // calculates v[0]^2 + v[1]^2 + v[2]^2, i.e. the norm squared
    float SquaredNorm(){
        let u[3]:float = {v[0]*v[0], v[1]*v[1], v[2]*v[2]};

        return Sum(u, 3);
    }
}

// computes the Euclidean dot product between two 3-dim vectors
auto DotProduct(v1:Vector, v2:Vector){
    let u[3]:auto = {v1.v[0] * v2.v[0], v1.v[1] * v2.v[1], v1.v[2] * v2.v[2]};
    return Sum(u, 3);
}

let v:Vector;
v.Set(0.125, 0.25, 0.5);
print v.SquaredNorm(); // 0.328125
print DotProduct(v, v) == v.SquaredNorm(); // true

let u:Vector;
u.Set(1.0, 1.0, 1.0);
print DotProduct(u, v); // 0.875
```

## Requirements

For installation, ```cmake``` version 3.17+ is required. The project makes use of the ```variant``` tagged union container
introduced in ```C++17```; ```gcc``` version 8+ has full support for ```C++17```.

## Installation Instructions

Clone the repository, and ```cd``` into the project directory. Then run:

1. ```cmake .```
2. ```make```

## Usage Instructions

Simply ```cd``` into the directory containing the compiled executable, and run ```./TeaLang2 /path/to/source_file.tlg [-v=1]```,
where ```/path/to/source_file.tlg``` is a required argument specifying the source file ending with the ```.tlg``` extension
and ```-v=1``` is an optional argument which when specified outputs a DOT file representing the AST of the source file.