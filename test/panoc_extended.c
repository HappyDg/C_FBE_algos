#include<stdio.h>
#include<math.h>

#include"../globals/globals.h"
#include"example_problems.h"
#include"../include/optimizer.h"

static int test(void);
static real_t rosen_cost_gradient(const real_t* input,real_t* output);

int main(){
    return test();
}

/*
 * Use the example of the Casadi dummy solver
 * Solve the Rosenbrock problem, formulated as the NLP:
 * minimize     x^2+tanh(y)^2
 *  x, y
 * subject to   cos(x+y)+0.5 == 0
 *              sin(x) + 0.5 <= 0
 * (copied from Joris Gillis, 2018, Casadi)
*/ 


/*
 * constraints with gradient
 * h_1(x,y) = cos(x+y)+0.5
 * dh_1(x,y)/dx = -sin(x+y)
 * dh_1(x,y)/dy = -sin(x+y)
 * 
 * h_2(x,y) = sin(x) + 0.5
 * dh_2(x,y)/dx = cos(x)
 * dh_2(x,y)/dy = 0
 */
static int constraints(const real_t* x,real_t* out){
    out[0] = cos(x[0]+x[1])+0.5;
    out[1] = sin(x[0]) + 0.5;

    return SUCCESS;
}
static int constraints_forward_diff(const real_t* x,const real_t* y,real_t* out){
    out[0]=-sin(x[0]+x[1])*y[0]+ -sin(x[0]+x[1])*y[1];
    out[1]=cos(x[0])*y[0]+0*y[1];

    return SUCCESS;
}

static int test(void){
    real_t lower_bound = -2;
    real_t upper_bound = 2;
    real_t constraint_weight_= 1;

    /*
     * prepare constraint
     */
    real_t low_bounds_constraints[2]={0,-INFINITY};
    real_t upper_bounds_constraints[2]={0,0};
    
    /*
     * set values related to cost function
     */
    struct optimizer_extended_problem extended_problem;
    extended_problem.problem.cost_gradient_function=rosen_cost_gradient;
    extended_problem.problem.dimension=2;

    /*
     * set constraints
     */
    extended_problem.lower_bounds_constraints=low_bounds_constraints;
    extended_problem.upper_bounds_constraints=upper_bounds_constraints;
    extended_problem.constraints=constraints;
	extended_problem.number_of_constraints = 2;
    extended_problem.constraints_forwad_diff=constraints_forward_diff;

    /*
     * set PANOC related values
     */
    extended_problem.max_loops=10;
    extended_problem.problem.solver_params.tolerance=1e-12;
    extended_problem.problem.solver_params.buffer_size=20;
    extended_problem.problem.solver_params.max_interations=200;

    optimizer_init_extended_box(&extended_problem,lower_bound,upper_bound,constraint_weight_);

    real_t solution[2]={0,0};
    int iterations = solve_extended_problem(solution);

    printf("solution in %d iterations = [%f ; %f] \n",iterations,solution[0],solution[1]);

	real_t error = ABS(solution[0] - 0.);
    if(error > extended_problem.problem.solver_params.tolerance)
        return FAILURE;
	error = ABS(solution[1] - 0.);
    if(error > extended_problem.problem.solver_params.tolerance)
        return FAILURE;

    optimizer_cleanup();
    return SUCCESS;
}

/*
 * f(x,y) = x^2+tanh(y)^2
 * gradient(x,y) /dx = 2x
 * gradient(x,y) /dy = (2*sinh(y))/(cosh(y))^3
 */
static real_t rosen_cost_gradient(const real_t* input,real_t* gradient){
    gradient[0]=input[1]*2;
    gradient[1]= (2*sinh(input[1]))/(cosh(input[1])*cosh(input[1])*cosh(input[1]));

    return input[0]*input[0] + tanh(input[1])*tanh(input[1]);
}