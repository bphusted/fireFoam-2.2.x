/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2009 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

\*---------------------------------------------------------------------------*/

#include "fixedIncidentRadiationFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "constants.H"
#include "radiationModel.H"
#include "absorptionEmissionModel.H"

#include "pyroCUPOneDimV1.H"
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{


using namespace Foam::constant;

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

fixedIncidentRadiationFvPatchScalarField::
fixedIncidentRadiationFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), "undefined", "undefined-K"),
    QrIncident_(p.size(), 0.0)
{}


fixedIncidentRadiationFvPatchScalarField::
fixedIncidentRadiationFvPatchScalarField
(
    const fixedIncidentRadiationFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    fixedGradientFvPatchScalarField(ptf, p, iF, mapper),
    temperatureCoupledBase(patch(), ptf.KMethod(), ptf.kappaName()),
    QrIncident_(ptf.QrIncident_)
{}


fixedIncidentRadiationFvPatchScalarField::
fixedIncidentRadiationFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    fixedGradientFvPatchScalarField(p, iF),
    temperatureCoupledBase(patch(), dict),
    QrIncident_("QrIncident", dict, p.size())
{
    if (dict.found("value") && dict.found("gradient"))
    {
        fvPatchField<scalar>::operator=(Field<scalar>("value", dict, p.size()));
        gradient() = Field<scalar>("gradient", dict, p.size());
    }
    else
    {
        // Still reading so cannot yet evaluate. Make up a value.
        fvPatchField<scalar>::operator=(patchInternalField());
        gradient() = 0.0;
    }
}


fixedIncidentRadiationFvPatchScalarField::
fixedIncidentRadiationFvPatchScalarField
(
    const fixedIncidentRadiationFvPatchScalarField& psf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    fixedGradientFvPatchScalarField(psf, iF),
    temperatureCoupledBase(patch(), psf.KMethod(), psf.kappaName()),
    QrIncident_(psf.QrIncident_)
{}


fixedIncidentRadiationFvPatchScalarField::
fixedIncidentRadiationFvPatchScalarField
(
    const fixedIncidentRadiationFvPatchScalarField& ptf
)
:
    fixedGradientFvPatchScalarField(ptf),
    temperatureCoupledBase(patch(), ptf.KMethod(), ptf.kappaName()),
    QrIncident_(ptf.QrIncident_)
{}

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //


void fixedIncidentRadiationFvPatchScalarField::autoMap
(
    const fvPatchFieldMapper& m
)
{
    fixedGradientFvPatchScalarField::autoMap(m);
    QrIncident_.autoMap(m);
}


void fixedIncidentRadiationFvPatchScalarField::rmap
(
    const fvPatchScalarField& ptf,
    const labelList& addr
)
{
    fixedGradientFvPatchScalarField::rmap(ptf, addr);

    const fixedIncidentRadiationFvPatchScalarField& thftptf =
        refCast<const fixedIncidentRadiationFvPatchScalarField>
        (
            ptf
        );

    QrIncident_.rmap(thftptf.QrIncident_, addr);
}


void fixedIncidentRadiationFvPatchScalarField::updateCoeffs()
{
    if (updated())
    {
        return;
    }
    
    scalarField intFld = patchInternalField();
    
    const radiation::radiationModel& radiation =
        db().lookupObject<radiation::radiationModel>("radiationProperties");

           
            HashTable<const Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1*> models =
                patch().boundaryMesh().mesh().time().lookupClass<Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1>();

            forAllConstIter(HashTable<const Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1*>, models, iter)
            {
                if (iter()->regionMesh().name() == patch().boundaryMesh().mesh().name())
                {
                   Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1& CUPModelObj = const_cast<Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1&>(*iter());
                   CUPModelObj.setQrad(QrIncident_,patch().index());
                   CUPModelObj.setQconv(scalarField(patch().size(),0.0),patch().index());
                   //iter()->setQrad(QrIncident_,patch().index());
                 //pyroModelType&  pyroModelObj = const_cast<pyroModelType&>(pyroModel(mesh,mesh.name()));

                //  Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1& CUPModelObj = dynamic_cast<Foam::regionModels::pyrolysisModels::pyroCUPOneDimV1&>(pyroModelObj);
             //CUPModelObj.setQrad(radField,patch().index());
             //CUPModelObj.setQconv(-nbrConvFlux,patch().index());
                }
            }



//Info << "emissivity is " << radiation.absorptionEmission().a() << endl;


            scalarField temissivity
            (
                radiation.absorptionEmission().e()().boundaryField()
                [
                    //nbrPatch.index()
                    patch().index()
                ]
            );

//Info << "emissivity_ is " << temissivity << endl;
    gradient() =
        //radiation.absorptionEmission().a()*
        temissivity*
        (
            QrIncident_
//          - physicoChemical::sigma.value()*pow4(*this)
          - physicoChemical::sigma.value()*pow4(intFld)  //use internal cell T         
        )/kappa(*this);



    fixedGradientFvPatchScalarField::updateCoeffs();

    if (debug)
    {
        scalar Qr = gSum(kappa(*this)*gradient()*patch().magSf());
        Info<< patch().boundaryMesh().mesh().name() << ':'
            << patch().name() << ':'
            << this->dimensionedInternalField().name() << " -> "
            << " radiativeFlux:" << Qr
            << " walltemperature "
            << " min:" << gMin(*this)
            << " max:" << gMax(*this)
            << " avg:" << gAverage(*this)
            << endl;
    }
}


void fixedIncidentRadiationFvPatchScalarField::write
(
    Ostream& os
) const
{
    fixedGradientFvPatchScalarField::write(os);
    temperatureCoupledBase::write(os);
    QrIncident_.writeEntry("QrIncident", os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    fixedIncidentRadiationFvPatchScalarField
);


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam


// ************************************************************************* //
