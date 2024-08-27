# "Clean" Code, Horrible Performance

## Polymorphism (If/Switch Bad)

### What is it

Polymorphism is an object-oriented programming (OOP) principle where a single interface or method can represent different underlying forms (data types). It allows for code that is cleaner, easier to maintain, and more extensible. The classic example is having a base class with a method that can be overridden by derived classes, allowing different behaviors based on the object type without changing the code that calls the method.

### Example

This is an example without Polymorphism. It uses `if`/`switch` statements.

```c
enum ShapeType { CIRCLE, RECTANGLE };

struct Shape {
    ShapeType type;
    double radius;  // for circle
    double width, height;  // for rectangle
};

double calculateArea(Shape* shape) {
    switch (shape->type) {
        case CIRCLE:
            return 3.14159 * shape->radius * shape->radius;
        case RECTANGLE:
            return shape->width * shape->height;
        default:
            return 0;
    }
}
```

This is an example using Polymorphism.

```c
#include <stdio.h>
#include <stdlib.h>

// Base struct for Shape
typedef struct Shape {
    double (*calculateArea)(struct Shape*);  // Function pointer for calculating area
} Shape;

// Circle struct, inheriting Shape
typedef struct {
    Shape base;  // Inheritance simulation
    double radius;
} Circle;

// Rectangle struct, inheriting Shape
typedef struct {
    Shape base;  // Inheritance simulation
    double width, height;
} Rectangle;

// Function to calculate area of a Circle
double calculateCircleArea(Shape* shape) {
    Circle* circle = (Circle*)shape;  // Cast to Circle
    return 3.14159 * circle->radius * circle->radius;
}

// Function to calculate area of a Rectangle
double calculateRectangleArea(Shape* shape) {
    Rectangle* rectangle = (Rectangle*)shape;  // Cast to Rectangle
    return rectangle->width * rectangle->height;
}

// Function to create a new Circle
Circle* createCircle(double radius) {
    Circle* circle = (Circle*)malloc(sizeof(Circle));
    circle->radius = radius;
    circle->base.calculateArea = calculateCircleArea;
    return circle;
}

// Function to create a new Rectangle
Rectangle* createRectangle(double width, double height) {
    Rectangle* rectangle = (Rectangle*)malloc(sizeof(Rectangle));
    rectangle->width = width;
    rectangle->height = height;
    rectangle->base.calculateArea = calculateRectangleArea;
    return rectangle;
}

// Main function to demonstrate polymorphism
int main() {
    Shape* shapes[2];
    shapes[0] = (Shape*)createCircle(5.0);
    shapes[1] = (Shape*)createRectangle(4.0, 6.0);

    for (int i = 0; i < 2; ++i) {
        printf("Area: %.2f\n", shapes[i]->calculateArea(shapes[i]));
        free(shapes[i]);
    }

    return 0;
}
```

## (No) Internals (For Things)

The principle of "No Internals for Things" is all about **keeping the inner workings of your code hidden** and only showing what's necessary to the outside world. This makes your code easier to maintain, understand, and change without breaking other parts of your program.

### Key Concepts:

1. **Encapsulation:**
   - Encapsulation means **hiding the details** of how something works inside a function or a structure. Instead of letting everyone see and mess with the internal parts, you provide a clean and simple way (like a public function) for others to interact with your code.

2. **Information Hiding:**
   - By keeping the internal details hidden, you can change how something works on the inside without affecting other parts of your program. This makes your code more stable and easier to update.

3. **Public API vs. Internal Details:**
   - The **Public API** is what you expose to the outside world—functions and data that others can use.
   - **Internal Details** are the things you keep hidden—private functions, variables, or complex logic that others don’t need to see or use directly.

4. **Benefits:**
   - **Reduced Dependencies:** Other parts of your code won’t rely on specific internal details, making everything more independent and easier to change.
   - **Easier Maintenance:** Since others don’t see or use the internal parts, you can change those parts without breaking anything.
   - **Clearer Code:** By showing only what’s necessary, your code becomes easier to understand and use.

### Simple Example in C

Let’s look at a basic example in C:

```c
#include <stdio.h>
#include <string.h>

// Define a Person type
typedef struct {
    char name[50];
    int age;
} Person;

// Public functions to interact with Person
void initializePerson(Person* p, const char* name, int age);
void printPersonInfo(const Person* p);

// Internal function to validate age (not exposed)
static void validateAge(int age) {
    if (age < 0 || age > 150) {
        printf("Invalid age provided.\n");
    }
}

// Public function to initialize a Person
void initializePerson(Person* p, const char* name, int age) {
    validateAge(age);  // Use internal function for validation
    strncpy(p->name, name, sizeof(p->name));
    p->age = age;
}

// Public function to print Person's info
void printPersonInfo(const Person* p) {
    printf("Name: %s\n", p->name);
    printf("Age: %d\n", p->age);
}

int main() {
    Person person;
    initializePerson(&person, "John Doe", 30);
    printPersonInfo(&person);
    return 0;
}
```

### Explanation:

- **Encapsulation:** The `validateAge()` function checks if the age is valid, but it’s marked as `static`, meaning it’s hidden from the outside world. Only the code inside this file can use it.
  
- **Public API:** The functions `initializePerson()` and `printPersonInfo()` are the only parts that other parts of your program can see and use. This is the public interface of your code.

- **No Internals for Things:** By keeping `validateAge()` hidden, you can change how age is validated later without worrying about breaking other parts of your program. Only the public functions are exposed, making your code more secure and easier to maintain.

### Summary:

The idea of "No Internals for Things" is simple: **hide the complex details** and only expose what others need to use. This keeps your code clean, makes it easier to change, and prevents other parts of your program from becoming dependent on the internal workings of your code.

## (Functions should be) Small

The concept of "Functions Should Be Small" is a key principle in writing clean, maintainable code. It suggests that functions should be short and focused on doing one thing well. This makes your code easier to read, understand, and maintain.

### Why Small Functions?

1. **Readability:**
   - Small functions are easier to read and understand. When you see a function that is only a few lines long, it’s clear what that function does without needing to dig through a lot of code.

2. **Single Responsibility:**
   - Each function should do one thing and do it well. This follows the Single Responsibility Principle (SRP), meaning each function should have one clear purpose. When a function is small, it's more likely to stick to doing just one thing.

3. **Easier to Debug:**
   - Small functions are easier to test and debug because you can pinpoint exactly where something went wrong. If a bug arises, it's easier to isolate the problem within a small function than within a large, complex one.

4. **Reusability:**
   - When functions are small and focused, they can often be reused in different parts of your program. This reduces duplication and makes your code more efficient.

5. **Simpler Maintenance:**
   - Code is easier to maintain when it’s broken down into small, manageable pieces. If you need to make a change, you only need to update a small, focused function rather than a large block of code.

### Example in C

Let’s say we want to calculate the area of a rectangle and print it. Here's how you might do it with small functions:

```c
#include <stdio.h>

// Small function to calculate the area of a rectangle
double calculateArea(double width, double height) {
    return width * height;
}

// Small function to print the area
void printArea(double area) {
    printf("The area is: %.2f\n", area);
}

// Main function that uses the small functions
int main() {
    double width = 5.0;
    double height = 3.0;
    
    double area = calculateArea(width, height);
    printArea(area);

    return 0;
}
```

### Explanation:

- **calculateArea Function:**
  - This function has one job: to calculate the area of a rectangle. It’s small and focused, making it easy to understand and reuse.

- **printArea Function:**
  - This function’s only job is to print the area. Again, it’s small and does just one thing.

- **Main Function:**
  - The `main()` function is clean and easy to read because it uses small functions to do the work. It just coordinates the actions without getting bogged down in details.

## (Functions should only do) 1 Thing

The principle that "Functions Should Only Do 1 Thing" is about ensuring each function in your code is focused on a single task or responsibility. This concept is closely related to the idea of keeping functions small, but it emphasizes the importance of clarity and simplicity in what a function does.

### Why Should Functions Only Do 1 Thing?

1. **Clarity and Simplicity:**
   - When a function does only one thing, it’s much easier to understand. You can look at the function and immediately know its purpose without needing to read through a lot of code.

2. **Easy to Maintain:**
   - Functions that do just one thing are easier to update or fix. If a change is needed, you only need to modify a small, focused part of your code, reducing the risk of introducing bugs.

3. **Improved Reusability:**
   - A function that does one thing is more likely to be reusable in different parts of your program. Since it's focused on a single task, you can use it wherever that task is needed.

4. **Better Testing:**
   - Testing is simpler when functions do just one thing. You can write more precise tests, knowing exactly what the function should do and not having to account for multiple tasks being handled in a single function.

5. **Adheres to Single Responsibility Principle (SRP):**
   - The idea that a function should have only one reason to change is a core part of the Single Responsibility Principle (SRP). By ensuring a function only does one thing, you make it easier to manage changes and updates.

### Example in C

Let’s look at an example where we split tasks into separate functions to ensure each does just one thing.

### Example: Reading, Calculating, and Printing

Suppose you want to read input, calculate the area of a rectangle, and print the result. Instead of doing everything in one function, you can break it down:

```c
#include <stdio.h>

// Function to read dimensions from the user
void readDimensions(double* width, double* height) {
    printf("Enter width: ");
    scanf("%lf", width);
    printf("Enter height: ");
    scanf("%lf", height);
}

// Function to calculate the area
double calculateArea(double width, double height) {
    return width * height;
}

// Function to print the area
void printArea(double area) {
    printf("The area is: %.2f\n", area);
}

// Main function that uses the smaller functions
int main() {
    double width, height;
    
    readDimensions(&width, &height);
    double area = calculateArea(width, height);
    printArea(area);

    return 0;
}
```

### Explanation:

- **readDimensions Function:**
  - This function has a single responsibility: to read the width and height from the user. It does nothing else.

- **calculateArea Function:**
  - This function’s only job is to calculate the area of the rectangle. It doesn’t worry about reading input or printing output.

- **printArea Function:**
  - This function only prints the calculated area. It doesn’t involve itself with reading data or doing calculations.


### D(on't) R(epeat) Y(ourself)

The **Don't Repeat Yourself (DRY)** principle is one of the foundational concepts in clean code. It suggests that you should avoid duplicating code or logic across your program. Instead of repeating the same code or logic in multiple places, you should abstract it into a single, reusable component.

#### Why is DRY Important?

1. **Reduced Maintenance Effort:**
   - When you have the same code in multiple places, any change or bug fix needs to be made in all those places. This increases the risk of missing something and introduces inconsistencies. By following the DRY principle, you only need to update your code in one place, making maintenance much easier.

2. **Improved Code Readability:**
   - Duplicated code makes it harder to understand your program. When logic is scattered across different parts of the codebase, it becomes challenging to see the bigger picture. By keeping your code DRY, you make it more concise and easier to follow.

3. **Consistency:**
   - Repeated code can lead to subtle variations, which can cause bugs and inconsistencies. DRY ensures that your program behaves consistently because there's only one version of the logic or data.

4. **Reusability:**
   - When you avoid duplication, you often end up creating reusable functions, classes, or modules. These components can be used in different parts of your program or even in different projects, increasing efficiency.

### Example in C

Let's consider an example where you calculate the area of a rectangle and a circle. Without DRY, you might write the same code multiple times:

#### Without DRY

```c
#include <stdio.h>

int main() {
    // Calculating area of a rectangle
    double width = 5.0;
    double height = 3.0;
    double rectangleArea = width * height;
    printf("Rectangle Area: %.2f\n", rectangleArea);

    // Calculating area of a circle
    double radius = 4.0;
    double circleArea = 3.14159 * radius * radius;
    printf("Circle Area: %.2f\n", circleArea);

    // Another rectangle area calculation
    double width2 = 7.0;
    double height2 = 2.0;
    double rectangleArea2 = width2 * height2;
    printf("Another Rectangle Area: %.2f\n", rectangleArea2);

    return 0;
}
```

In the above code, the logic for calculating the area of a rectangle is repeated twice. This is not following the DRY principle.

#### With DRY

Now, let's refactor the code to follow the DRY principle by creating reusable functions:

```c
#include <stdio.h>

// Function to calculate the area of a rectangle
double calculateRectangleArea(double width, double height) {
    return width * height;
}

// Function to calculate the area of a circle
double calculateCircleArea(double radius) {
    return 3.14159 * radius * radius;
}

int main() {
    // Using the function to calculate rectangle area
    double width = 5.0;
    double height = 3.0;
    double rectangleArea = calculateRectangleArea(width, height);
    printf("Rectangle Area: %.2f\n", rectangleArea);

    // Using the function to calculate circle area
    double radius = 4.0;
    double circleArea = calculateCircleArea(radius);
    printf("Circle Area: %.2f\n", circleArea);

    // Reusing the function to calculate another rectangle area
    double width2 = 7.0;
    double height2 = 2.0;
    double rectangleArea2 = calculateRectangleArea(width2, height2);
    printf("Another Rectangle Area: %.2f\n", rectangleArea2);

    return 0;
}
```

### Explanation:

- **calculateRectangleArea Function:** This function encapsulates the logic for calculating the area of a rectangle. Instead of duplicating the multiplication code, we reuse this function wherever we need to calculate a rectangle’s area.
  
- **calculateCircleArea Function:** Similarly, this function handles the area calculation for a circle. The logic is only written once and reused.

### Benefits:

- **Reduced Duplication:** The logic for calculating areas is not repeated. Instead, it's centralized in reusable functions.
- **Simplified Maintenance:** If the formula for area calculation needs to change, you only need to update it in one place.
- **Increased Reusability:** The functions can be used anywhere in the program, reducing the need to write similar code repeatedly.