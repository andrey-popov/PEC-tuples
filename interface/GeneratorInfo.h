#pragma once

#include <Rtypes.h>

#include <vector>


namespace pec
{
/**
 * \class GeneratorInfo
 * \brief Aggregates basic generator-level information
 */
class GeneratorInfo
{
public:
    /// Constructor without parameters
    GeneratorInfo() noexcept;
    
    /// Default copy constructor
    GeneratorInfo(GeneratorInfo const &) = default;
    
    /// Default assignment operator
    GeneratorInfo &operator=(GeneratorInfo const &) = default;
    
public:
    /// Resets the object to a state right after the default initialisation
    void Reset();
    
    /**
     * \brief Sets process ID
     * 
     * See documentation for the method ProcessID for details.
     */
    void SetProcessId(int processID);
    
    /// Sets the nominal generator-level weight
    void SetNominalWeight(float weight);
    
    /// Adds an alternative LHE event weight to the end of the collection
    void AddAltLheWeight(float weight);
    
    /// Adds an alternative PS event weight to the end of the collection
    void AddAltPsWeight(float weight);
    
    /**
     * \brief Sets momentum fraction carried by an initial parton
     * 
     * Throws an exception if the given index is large than or the fraction is illegal.
     */
    void SetPdfX(unsigned index, float x);
    
    /**
     * \brief Sets momentum fractions for both initial partons simultaneously
     * 
     * Internally, calls to SetPdfX, check documentation for this method.
     */
    void SetPdfXs(float x1, float x2);
    
    /**
     * \brief Sets ID of an initial parton
     * 
     * Gluons must be encoded with codes 0 or 21 (both are accepted). If the index is larger than 1,
     * an exception is thrown.
     */
    void SetPdfId(unsigned index, int id);
    
    /**
     * \brief Sets IDs of both initial partons
     * 
     * Internally, calls to SetPdfId, check documentation for this method.
     */
    void SetPdfIds(int id1, int id2);
    
    /// Sets energy scale used to evaluate PDF, GeV
    void SetPdfQScale(float scale);
    
    /**
     * \brief Returns process ID
     * 
     * This is supposed to be the process ID set be the generator. For instance, different
     * subprocesses in MadGraph are assigned different process IDs.
     */
    int ProcessId() const;
    
    /// Returns the nominal generator-level weight
    float NominalWeight() const;
    
    /// Returns alternative LHE weights
    std::vector<Float_t> const &AltLheWeights() const;

    /// Returns alternative PS weights
    std::vector<Float_t> const &AltPsWeights() const;
    
    /**
     * \brief Returns momentum fraction carried by an initial parton
     * 
     * Throws an exception if the index is larger than 1.
     */
    float PdfX(unsigned index) const;
    
    /**
     * \brief Returns ID of an initial parton
     * 
     * Gluons are encoded with their PDG ID code (21). If the index is larger than 1, an exception
     * is thrown.
     */
    int PdfId(unsigned index) const;
    
    /// Returns energy scale used to evaluate PDF, GeV
    float PdfQScale() const;
    
private:
    /// Process ID as was set during generation of the sample
    Short_t processId;
    
    /// Nominal generator-level weight
    Float_t nominalWeight;
    
    /// Alternative LHE weights
    std::vector<Float_t> altLheWeights;

    /// Alternative PS weights
    std::vector<Float_t> altPsWeights;
    
    /// Momenta fractions carried by initial-state partons
    Float_t pdfX[2];
    
    /**
     * \brief ID of initial-state partons
     * 
     * The two are encoded in a single byte. First ID is (pdfID % 16 - 5), the second is
     * (pdf / 16 - 5). Gluons are encoded with zeros.
     */
    UChar_t pdfId;
    
    /// Energy scale to evaluate PDF, GeV
    Float_t pdfQScale;
};
}  // end of namespace pec
