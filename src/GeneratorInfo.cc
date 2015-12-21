#include <Analysis/PECTuples/interface/GeneratorInfo.h>
#include <Analysis/PECTuples/interface/minifloats.h>

#include <algorithm>
#include <stdexcept>


using namespace pec;


GeneratorInfo::GeneratorInfo():
    processId(0),
    pdfX(),  // the array is zeroed according to the C++03 standard
    pdfId(0),
    pdfQScale(0)
{}


GeneratorInfo::GeneratorInfo(GeneratorInfo const &src):
    processId(src.processId),
    weights(src.weights),
    pdfId(src.pdfId),
    pdfQScale(src.pdfQScale)
{
    std::copy(src.pdfX, src.pdfX + 2, pdfX);
}


GeneratorInfo &GeneratorInfo::operator=(GeneratorInfo const &src)
{
    processId = src.processId;
    weights = src.weights;
    std::copy(src.pdfX, src.pdfX + 2, pdfX);
    pdfId = src.pdfId;
    pdfQScale = src.pdfQScale;
    
    return *this;
}


void GeneratorInfo::Reset()
{
    processId = 0;
    weights.clear();
    pdfId = 0;
    pdfX[0] = pdfX[1] = 0;
    pdfQScale = 0;
}


void GeneratorInfo::SetProcessId(int processId_)
{
    processId = processId_;
}


void GeneratorInfo::AddWeight(double weight_)
{
    weights.emplace_back(minifloat::encodeGeneric<true, 10, 14>(weight_));
}


void GeneratorInfo::SetPdfX(unsigned index, double x)
{
    // Check the index
    if (index > 1)
        throw std::logic_error("GeneratorInfo::SetPdfX: Illegal parton index.");
    
    
    // Check the desired fraction
    if (x < 0. or x > 1.)
        throw std::logic_error("GeneratorInfo::SetPdfX: The fraction must be in the range "
         "[0., 1.].");
    
    
    // Set the fraction
    pdfX[index] = minifloat::encodeGeneric<false, 13, 7>(x);
}


void GeneratorInfo::SetPdfXs(double x1, double x2)
{
    SetPdfX(0, x1);
    SetPdfX(1, x2);
}


void GeneratorInfo::SetPdfId(unsigned index, int id)
{
    // Check the index
    if (index > 1)
        throw std::logic_error("GeneratorInfo::SetPdfId: Illegal parton index.");
    
    
    // If gluons are encoded with their PDG ID code (as in Pythia 8), change it to zero
    if (id == 21)
        id = 0;
    
    
    // Check the parton ID code
    if (abs(id) > 5)
        throw std::logic_error("GeneratorInfo::SetPdfId: Illegal parton ID.");
    
    
    // Set ID of the specified parton. The other one is not changed. The given ID is increased by 5
    //to make it non-negative
    if (index == 0)
        pdfId = (pdfId - pdfId % 16) + UChar_t(id + 5);
    else
        pdfId = 16 * UChar_t(id + 5) + pdfId % 16;
}


void GeneratorInfo::SetPdfIds(int id1, int id2)
{
    SetPdfId(0, id1);
    SetPdfId(1, id2);
}


void GeneratorInfo::SetPdfQScale(double scale)
{
    pdfQScale = minifloat::encodeGeneric<false, 12, 0>(scale);
}


int GeneratorInfo::ProcessId() const
{
    return processId;
}


unsigned GeneratorInfo::NumWeights() const
{
    return weights.size();
}


double GeneratorInfo::Weight(unsigned index) const
{
    if (index >= weights.size())
        throw std::range_error("GeneratorInfo::Weight: Index given is out of range.");
    
    return minifloat::decodeGeneric<true, 10, 14>(weights.at(index));
}


double GeneratorInfo::PdfX(unsigned index) const
{
    if (index > 1)
        throw std::logic_error("GeneratorInfo::PdfX: Illegal parton index.");
    
    return minifloat::decodeGeneric<false, 13, 7>(pdfX[index]);
}


int GeneratorInfo::PdfId(unsigned index) const
{
    if (index > 1)
        throw std::logic_error("GeneratorInfo::PdfId: Illegal parton index.");
    
    
    // Decode the parton ID
    int id;
    
    if (index == 0)
        id = int(pdfId) % 16 - 5;
        //^ Note the type conversion which is needed as otherwise the final result would be unsigned
    else
        id = int(pdfId) / 16 - 5;
    
    
    // Internally, gluons are encoded with code 0; return the PDG ID code for them instead
    if (id == 0)
        return 21;
    else
        return id;
}


double GeneratorInfo::PdfQScale() const
{
    return minifloat::decodeGeneric<false, 12, 0>(pdfQScale);
}
