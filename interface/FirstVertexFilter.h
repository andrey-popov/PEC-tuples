/**
 * @author Andrey.Popov@cern.ch
 *
 * The plugin performs string-based filtering on the first vertex in the collection given. The
 * vertices that pass the selection are put into the event content in a separate collection.
 * 
 * Usage example:
 *   process.goodOfflinePrimaryVertices = cms.EDFilter("FirstVertexFilter",
 *      src = cms.InputTag("offlinePrimaryVertices"),
 *      cut = cms.string("!isFake & ndof >= 4. & abs(z) < 24. & position.Rho < 2."))
 */

#ifndef SINGLE_TOP_FIRST_VERTEX_FILTER
#define SINGLE_TOP_FIRST_VERTEX_FILTER

#include <FWCore/Framework/interface/EDFilter.h>
#include <FWCore/Framework/interface/Event.h>
#include <FWCore/ParameterSet/interface/ParameterSet.h>
#include <FWCore/Utilities/interface/InputTag.h>

#include <string>


class FirstVertexFilter: public edm::EDFilter
{
	public:
		explicit FirstVertexFilter(const edm::ParameterSet &);
		~FirstVertexFilter();

	private:
		virtual void beginJob();
		virtual bool filter(edm::Event &, const edm::EventSetup &);
		virtual void endJob();

		edm::InputTag src;
		std::string cut;
};

#endif
