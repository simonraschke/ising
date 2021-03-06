#pragma once

#include "spin.hpp"
#include "lib/enhance.hpp"
#include "utility/histogram.hpp"
#include "utility/logger.hpp"
#include "gui/parameters/base_parameters_widget.hpp"
#include <ostream>
#include <string>
#include <sstream>



class Spinsystem
{
private:
    double Hamiltonian {0};
    std::vector<Spin> spins {};
    
    // Fuer Aufgabe 1.4:
    std::vector<unsigned int> lastFlipped {};   // contains spin-ID's of flipped Spins from last call to flip()

    void   computeHamiltonian();
    double localEnergyInteraction(const Spin&) const;
    double localEnergyMagnetic(const Spin&) const;

public:
    void flip();
    void flip_back();

    double getMagnetisation() const;
    auto   getHamiltonian() const { return Hamiltonian; }


/* 
 * DIE FOLGENDEN FUNKTIONEN SOLLTEN NUETZLICH FUER DIE
 * IMPLEMENTIERUNGSAUFGABEN SEIN
 * IHRE KONKRETE IMPLEMENTIERUNG MUSS ZUR VERWENDUNG DER FUNKTIONEN 
 * JEDOCH NICHT BEKANNT SEIN
 */ 
public:
    unsigned long getHeight() const;         // returns system height
    unsigned long getWidth() const;          // returns system width
    double        getInteraction() const;    // returns J
    double        getMagnetic()  const;      // returns B
    bool          getSpinExchange()  const;  // returns true if spin-exchange mode, else false

    std::string getStringOfSystem() const;   // returns a string with current spin configuration


/*
 * DER HIER FOLGENDE TEIL DER KLASSE IST NICHT RELEVANT FUER 
 * DIE IMPLEMENTIERUNGSAUFGABEN UND KANN IGNORIERT WERDEN 
 */ 
private:
    BaseParametersWidget* parameters = Q_NULLPTR;
    double distance(const Spin&, const Spin&) const;

public:
    Spinsystem()  {};
    ~Spinsystem() {};
    Spinsystem(const Spinsystem&) = delete;
    void operator=(const Spinsystem&) = delete;

    inline const auto& getSpins() const { return spins; };

    double        getRatio() const;              
    bool          getWavelengthPattern() const; 
    int           getWavelength() const;   

    void setParameters(BaseParametersWidget*);
    void setup();
    void resetParameters();
    void resetSpins();
    void resetSpinsCosinus(const double);

    Histogram<double> computeCorrelation() const;
    Histogram<double> computeStructureFunction(const Histogram<double>) const;
    // void computeSystemTimesCos() const;

    void print(std::ostream & ) const;

};



