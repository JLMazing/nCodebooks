
# original version by 'Tom Schaul, tom@idsia.ch'
# adapted so that it follows ask and tell structure



from scipy import array, dot, exp, log, sqrt, floor, ones, randn, zeros_like, Inf, argmax, eye, outer
from scipy.linalg import expm2

class xNES:


    def __init__(self, x0, maxEvals=1e6, verbose=True, targetFitness= -1e-10):
        self.maxEvals = maxEvals
        self.verbose = verbose
        self.targetFitness = targetFitness
        self.dim = len(x0)  
        self.I = eye(self.dim)
        self.learningRate = 0.6 * (3 + log(self.dim)) / self.dim / sqrt(self.dim)
        self.batchSize = 4 + int(floor(3 * log(self.dim)))    
        self.center = x0.copy()
        self.A = eye(self.dim)  # sqrt of the covariance matrix
        self.numEvals = 0
        self.bestFound = None
        self.bestFitness = -Inf

    def ask(self):
        samples = [randn(self.dim) for _ in range(self.batchSize)]
        samples = [(dot(self.A, s) + self.center) for s in samples]
        return samples


    

    def tell(self, samples, fitnesses):
        print fitnesses
        if max(fitnesses) > self.bestFitness:
            self.bestFitness = max(fitnesses)
            self.bestFound = samples[argmax(fitnesses)]
        self.numEvals += self.batchSize 
        if self.verbose: print "Step", self.numEvals / self.batchSize, ":", max(fitnesses), "best:", self.bestFitness
        #print A
          # update center and variances
        utilities = self.computeUtilities(fitnesses)
        self.center += dot(self.A, dot(utilities, samples))
        covGradient = sum([u * (outer(s, s) - self.I) for (s, u) in zip(samples, utilities)])
        self.A = dot(self.A, expm2(0.5 * self.learningRate * covGradient))                      

    def computeUtilities(self, fitnesses):
        L = len(fitnesses)
        ranks = zeros_like(fitnesses)
        l = zip(fitnesses, range(L))
        l.sort()
        for i, (_, j) in enumerate(l):
            ranks[j] = i
        # smooth reshaping
        utilities = array([max(0., x) for x in log(L / 2. + 1.0) - log(L - array(ranks))])
        utilities /= sum(utilities)       # make the utilities sum to 1
        utilities -= 1. / L  # baseline
        return utilities

       
#      def xNES(f, x0, maxEvals=1e6, verbose=False, targetFitness= -1e-10):
#          """ Exponential NES (xNES), as described in 
#        Glasmachers, Schaul, Sun, Wierstra and Schmidhuber (GECCO'10).
#        Maximizes a function f. 
#        Returns (best solution found, corresponding fitness).
#        """
#        dim = len(x0)  
#    I = eye(dim)
#    learningRate = 0.6 * (3 + log(dim)) / dim / sqrt(dim)
#    batchSize = 4 + int(floor(3 * log(dim)))    
#    center = x0.copy()
#    A = eye(dim)  # sqrt of the covariance matrix
#    numEvals = 0
#    bestFound = None
#    bestFitness = -Inf
#    while numEvals + batchSize <= maxEvals and bestFitness < targetFitness:
#        # produce and evaluate samples
#        samples = [randn(dim) for _ in range(batchSize)]
#        fitnesses = [f(dot(A, s) + center) for s in samples]
#        if max(fitnesses) > bestFitness:
#            bestFitness = max(fitnesses)
#            bestFound = samples[argmax(fitnesses)]
#        numEvals += batchSize 
#        if verbose: print "Step", numEvals / batchSize, ":", max(fitnesses), "best:", bestFitness
#        #print A
#        # update center and variances
#        utilities = computeUtilities(fitnesses)
#        center += dot(A, dot(utilities, samples))
#        covGradient = sum([u * (outer(s, s) - I) for (s, u) in zip(samples, utilities)])
#        A = dot(A, expm2(0.5 * learningRate * covGradient))                      
#
#    return bestFound, bestFitness
    
    

    
if __name__ == "__main__":
    from scipy import dot, array, power
    
    # Rosenbrock function
    def rosen(x):
      #  return - sum(100 * (x[:-1] ** 2 - x[1:]) ** 2 + (x[:-1] - 1) ** 2)

        return  sum(100 * (x[:-1] ** 2 - x[1:]) ** 2 + (x[:-1] - 1) ** 2)
    
    # example run (30-dimensional Rosenbrock)
    xnes = xNES(ones(2), verbose=True)
    for _ in range(1000):
        sol = xnes.ask()
        #print sol
        #print [(rosen(x)) for x in sol]
        xnes.tell(sol, [(rosen(x)) for x in sol])
        


