#include <UserCode/SingleTop/interface/GeneratorInfo.h>

#include <algorithm>
#include <stdexcept>


using namespace pec;


GeneratorInfo::GeneratorInfo():
    processId(0),
    weight(0),
    pdfX(),  // the array is zeroed according to the C++03 standard
    pdfId(0),
    pdfQScale(0)
{}


GeneratorInfo::GeneratorInfo(GeneratorInfo const &src):
    processId(src.processId),
    weight(src.weight),
    pdfId(src.pdfId),
    pdfQScale(src.pdfQScale)
{
    std::copy(src.pdfX, src.pdfX + 2, pdfX);
}


GeneratorInfo &GeneratorInfo::operator=(GeneratorInfo const &src)
{
    processId = src.processId;
    weight = src.weight;
    std::copy(src.pdfX, src.pdfX + 2, pdfX);
    pdfId = src.pdfId;
    pdfQScale = src.pdfQScale;
    
    return *this;
}


void GeneratorInfo::Reset()
{
    processId = 0;
    weight = 0;
    pdfId = 0;
    pdfX[0] = pdfX[1] = 0;
    pdfQScale = 0;
}


void GeneratorInfo::SetProcessId(int processId_)
{
    processId = processId_;
}


void GeneratorInfo::SetWeight(double weight_)
{
    weight = weight_;
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
    pdfX[index] = x;
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
    
    
    // Check the desired fraction
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
    pdfQScale = scale;
}


int GeneratorInfo::ProcessId() const
{
    return processId;
}


double GeneratorInfo::Weight() const
{
    return weight;
}


double GeneratorInfo::PdfX(unsigned index) const
{
    if (index > 1)
        throw std::logic_error("GeneratorInfo::PdfX: Illegal parton index.");
    
    return pdfX[index];
}


int GeneratorInfo::PdfId(unsigned index) const
{
    if (index > 1)
        throw std::logic_error("GeneratorInfo::PdfId: Illegal parton index.");
    
    if (index == 0)
        return int(pdfId) % 16 - 5;
        //^ Note the type conversion which is needed as otherwise the final result would be unsigned
    else
        return int(pdfId) / 16 - 5;
}


double GeneratorInfo::PdfQScale() const
{
    return pdfQScale;
}
