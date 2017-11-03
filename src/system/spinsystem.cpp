#include "spinsystem.hpp"


void Spinsystem::setParameters(ParametersWidget* prms)
{
    Q_CHECK_PTR(prms);
    parameters = prms;
    Q_CHECK_PTR(parameters);
}




void Spinsystem::setup()
{
    Q_CHECK_PTR(parameters);

    spins.clear();
    lastFlipped.clear();
    correlation.clear();

    // safety check:
    if( parameters->getConstrained() && (parameters->getWidth()*parameters->getHeight()) % 2 != 0 )
    {
        throw std::logic_error("system size must be an even number if system is constrained");
    }
    if( parameters->getConstrained() && parameters->getMagnetic() != 0 )
    {
        throw std::logic_error("constrained system cannot have a magnetic field");
    }

    auto width  = parameters->getWidth();
    auto height = parameters->getHeight();
    auto totalnumber = width * height;

    // set spinarray:
    int random;
    if( ! parameters->getConstrained() ) // initialise spins randomly
    {
        for(unsigned int i=0; i<totalnumber; ++i)
        {
            random = enhance::random_int(0,1);
            spins.emplace_back(i, random == 1 ? UP : DOWN );
        }
    }                   // constrained to specific up-spin to down-spin ratio
    else
    {
        for(unsigned int i=0; i<totalnumber; ++i)
            spins.emplace_back(i, UP);
        for(unsigned int i=0; i<static_cast<unsigned int>(parameters->getRatio() * totalnumber); ++i)
        {
            do
            {
                random = enhance::random_int(0, totalnumber-1);
            }
            while( spins[random].get_type() == DOWN );
            spins[random].set_type(DOWN);
        }
    }

    // set neighbours:
    for(auto& s: spins)
    {
        std::vector<std::reference_wrapper<Spin> > Nrefs;
        unsigned int Nid;
        const unsigned int id = s.get_ID();

        {
            // up
            Nid = ((long)id - static_cast<long>(width)) < 0 ? id - width + totalnumber : id - width;
            assert( Nid < spins.size() );
            if( Nid != id )
                Nrefs.push_back( std::ref(spins[Nid]) );
        }

        {
            // right
            Nid = (id + 1) % width == 0  ? id + 1 - width : id + 1;
            assert( Nid < spins.size() );
            if( Nid != id )
                Nrefs.push_back( std::ref(spins[Nid]) );
        }

        {
            // below
            Nid = id + width >= totalnumber ? id + width - totalnumber : id + width;
            assert( Nid < spins.size() );
            if( Nid != id )
                 Nrefs.push_back( std::ref(spins[Nid]) );
        }

        {
            // left
            Nid = id % width == 0  ? id - 1 + width : id - 1;
            assert( Nid < spins.size() );
            if( Nid != id )
                 Nrefs.push_back( std::ref(spins[Nid]) );
        }

        s.set_neighbours(Nrefs);
    }
    // debugging:
    for(auto& s: spins)
    {
        qDebug() << "spin" << s.get_ID() << "has neighbours :";
        std::for_each( s.begin(), s.end(), [](const auto& N){qDebug() << N.get().get_ID() << " ";} );
    }
    qDebug() << ' ';

    // calculate initial Hamiltonian:
    Hamiltonian = std::accumulate(std::cbegin(spins), std::cend(spins), static_cast<double>(0), [&](double i, const Spin& S)
                        {
                            return i + local_energy(S);
                        }) / 2;
    qDebug() << "initial H = " << Hamiltonian;
    qDebug() << ' ';

}

/***************************************************************************/

void Spinsystem::randomise() 
{
    // randomly set types of all spins new
    int random;
    if( ! parameters->getConstrained() ) // initialise spins randomly
    {
        for( auto& s: spins )
        {
            random = enhance::random_int(0,1);
            s.set_type( random == 1 ? UP : DOWN );
        }
    }                   // constrained to specific up-spin to down-spin ratio
    else
    {
        for( auto& s: spins ) 
            s.set_type( UP );
        for(unsigned int i=0; i<static_cast<unsigned int>(parameters->getRatio() * spins.size()); ++i)
        {
            do
            {
                random = enhance::random_int(0, spins.size()-1);
            }
            while( spins[random].get_type() == DOWN );
            spins[random].set_type(DOWN);
        }
    }

    // clear / reset all vectors: 
    lastFlipped.clear();
    correlation.reset();
    
    // calculate initial Hamiltonian:
    Hamiltonian = std::accumulate(std::cbegin(spins), std::cend(spins), static_cast<double>(0), [&](double i, const Spin& S)
    {
        return i + local_energy(S);
    }) / 2;
    qDebug() << "initial H = " << Hamiltonian;
    qDebug() << ' ';

}

/***************************************************************************/

double Spinsystem::local_energy(const Spin& _spin) const
{
    // calculate local energy for given spin
    
    double energy = - Jij(SPINTYPE::UP, _spin.get_type()) * _spin.num_signed<SPINTYPE::UP>()
                    - Jij(SPINTYPE::DOWN, _spin.get_type()) * _spin.num_signed<SPINTYPE::DOWN>();
    energy -= parameters->getMagnetic() * (_spin.get_type() == SPINTYPE::UP ? 2.f : -2.f);
    qDebug() << "spin # " << _spin.get_ID() << "  has local energy " << energy;
    return energy;
}

/***************************************************************************/

double Spinsystem::Jij(const SPINTYPE _spin1, const SPINTYPE _spin2) const
{
    // return correct J for this pair of spins depending on CONSTRAINED
    
    if( ! parameters->getConstrained() ) 
        return parameters->getInteraction();
    else                
        return _spin1 != _spin2 ? parameters->getInteraction() : 0;
}

/***************************************************************************/

void Spinsystem::flip()
{
    lastFlipped.clear();
    double localEnergy_before = 0.f;
    double localEnergy_after = 0.f;

    if( ! parameters->getConstrained() )
    {
        // find random spin
        auto randomspin = enhance::random_iterator(spins);
        lastFlipped.emplace_back( std::ref(*randomspin) );
        // flip spin
        localEnergy_before = local_energy( *randomspin );
        randomspin->flip();
        localEnergy_after = local_energy( *randomspin );
        // update Hamiltonian:
        Hamiltonian += localEnergy_after - localEnergy_before;
    }
    else
    {
        // find random spin
        auto randomspin = enhance::random_iterator(spins);
        do
        {
            randomspin = enhance::random_iterator(spins);
        } while( randomspin->num_opposite() == 0 );
        
        // find random neighbour
        auto randomneighbour = enhance::random_iterator(randomspin->get_neighbours());
        do
        {
                randomneighbour = enhance::random_iterator(randomspin->get_neighbours());
        } while( randomneighbour->get().get_type() == randomspin->get_type() );
        
        // flip spins
        lastFlipped.emplace_back(*randomspin);
        lastFlipped.emplace_back(randomneighbour->get());
        localEnergy_before = local_energy(*randomspin) + local_energy(randomneighbour->get());
        randomspin->flip();
        randomneighbour->get().flip();
        localEnergy_after = local_energy(*randomspin) + local_energy(randomneighbour->get());

        // update Hamiltonian
        Hamiltonian += localEnergy_after - localEnergy_before;
    }

#ifndef NDEBUG
    qDebug() << "flipping spin: ";
    for(const auto& s: lastFlipped) qDebug() << s.get().get_ID() << " ";
    qDebug() << ' ';
#endif
}

/***************************************************************************/

void Spinsystem::flip_back()
{
    if( lastFlipped.size() == 0 )
        throw std::logic_error("Cannot flip back, since nothing has flipped yet");
    else
    {
        int localEnergy_before = 0;
        int localEnergy_after = 0;

        for( const auto& s: lastFlipped ) localEnergy_before += local_energy( s.get() );
        for( const auto& s: lastFlipped ) s.get().flip();
        for( const auto& s: lastFlipped ) localEnergy_after += local_energy( s.get() );

        // update Hamiltonian
        Hamiltonian += localEnergy_after - localEnergy_before;
    }

    qDebug() << "flipping back: ";
    for(const auto& s: lastFlipped) qDebug() << s.get().get_ID() << " ";
    qDebug() << ' ';

}

/***************************************************************************/

void Spinsystem::print(std::ostream & stream) const
{
    // print spinarray to stream
    for(const auto& s: spins)
    {
        stream << ( s.get_type() == DOWN ? "-" : "+" )
        << ( (static_cast<unsigned int long>(s.get_ID() + 1)) % parameters->getWidth() == 0 ? '\n' : ' ');
    }
}

/***************************************************************************/

std::string Spinsystem::str() const
{
    std::stringstream sstream;
    sstream << *this;
    
    return sstream.str(); 
}

/***************************************************************************/

const char* Spinsystem::c_str() const
{
    return str().c_str();
}

/***************************************************************************/

double Spinsystem::distance(const Spin& _spin1, const Spin& _spin2) const
{
    // compute distance between spins _spin1 and _spin2
    int a, b, c, d, x, y;

    a = _spin1.get_ID() % getWidth();
    b = _spin1.get_ID() / getWidth();
    c = _spin2.get_ID() % getWidth();
    d = _spin2.get_ID() / getWidth();


    x = (std::abs(c - a) <= static_cast<int>(getWidth()/2) ? std::abs(c - a) : std::abs(c - a) - getWidth());
    y = (std::abs(d - b) <= static_cast<int>(getHeight()/2) ? std::abs(d - b) : std::abs(d - b) - getHeight());

    return  std::sqrt( x*x + y*y );
}

/***************************************************************************/

void Spinsystem::correlate()
{
    // compute correlations between spins

    qDebug() << "computing correlation <Si Sj>:\n";
    correlation.clear();

    auto counter = correlation;
    double dist;

    for( auto s1 = std::begin(spins); s1 != std::end(spins); s1 += 1)
    {
        for( auto s2 = s1 + 1; s2 != std::end(spins); s2 += 1)
        {
            dist = distance(*s1, *s2);
            if( s1->get_type() == s2->get_type() )
            {
                if( ! correlation.contains(dist) )
                {
                    correlation.add_bin(dist);
                    counter.add_bin(dist);
                }
                correlation.add_data( dist );
                qDebug() << "correlating " << s1->get_ID() << " with " << s2->get_ID() << " : " << " 1 ";
            }
            else 
            {
                qDebug() << "correlating " << s1->get_ID() << " with " << s2->get_ID() << " : " << "   ";
            }
            counter.add_data( dist );
            qDebug() << " distance " << dist << '\n';
        }
    }

    // normalisation:
    std::for_each(std::begin(correlation), std::end(correlation), [&](auto& B)
    { 
        B.counter /= counter.get_data(B.value); 
    });

}