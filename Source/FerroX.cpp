#include "FerroX.H"

#include "Utils/SelectWarpXUtils/MsgLogger/MsgLogger.H"
#include "Utils/SelectWarpXUtils/WarnManager.H"
#include "Utils/SelectWarpXUtils/WarpXUtil.H"
#include "Utils/SelectWarpXUtils/WarpXProfilerWrapper.H"
#include "../../Utils/SelectWarpXUtils/WarpXUtil.H"

#include "Input/GeometryProperties/GeometryProperties.H"
#include "Input/BoundaryConditions/BoundaryConditions.H"
#include <AMReX_ParmParse.H>

c_FerroX* c_FerroX::m_instance = nullptr;
#ifdef AMREX_USE_GPU
bool c_FerroX::do_device_synchronize = true;
#else
bool c_FerroX::do_device_synchronize = false;
#endif

c_FerroX& c_FerroX::GetInstance() 
{

    if (!m_instance) {
        m_instance = new c_FerroX();
    }
    return *m_instance;

}


void
c_FerroX::ResetInstance ()
{
    delete m_instance;
    m_instance = nullptr;
}


c_FerroX::c_FerroX ()
{
#ifdef PRINT_NAME
    amrex::Print() << "\n\n\t{************************c_FerroX Constructor()************************\n";
#endif
    m_instance = this;
    m_p_warn_manager = std::make_unique<Utils::WarnManager>();

    ReadData();

#ifdef PRINT_NAME
    amrex::Print() << "\t}************************c_FerroX Constructor()************************\n";
#endif
}


c_FerroX::~c_FerroX ()
{
#ifdef PRINT_NAME
    amrex::Print() << "\n\n\t{************************c_FerroX Destructor()************************\n";
#endif

#ifdef PRINT_NAME
    amrex::Print() << "\t}************************c_FerroX Destructor()************************\n";
#endif
}


void
c_FerroX::RecordWarning(
        std::string topic,
        std::string text,
        WarnPriority priority)
{
    WARPX_PROFILE("WarpX::RecordWarning");

    auto msg_priority = Utils::MsgLogger::Priority::high;
    if(priority == WarnPriority::low)
        msg_priority = Utils::MsgLogger::Priority::low;
    else if(priority == WarnPriority::medium)
        msg_priority = Utils::MsgLogger::Priority::medium;

    if(m_always_warn_immediately){
        amrex::Warning(
            "!!!!!! WARNING: ["
            + std::string(Utils::MsgLogger::PriorityToString(msg_priority))
            + "][" + topic + "] " + text);
    }

#ifdef AMREX_USE_OMP
    #pragma omp critical
#endif
    {
        m_p_warn_manager->record_warning(topic, text, msg_priority);
    }
}


void
c_FerroX::PrintLocalWarnings(const std::string& when)
{

    WARPX_PROFILE("WarpX::PrintLocalWarnings");
    const std::string warn_string = m_p_warn_manager->print_local_warnings(when);
    amrex::AllPrint() << warn_string;

}


void
c_FerroX::PrintGlobalWarnings(const std::string& when)
{

    WARPX_PROFILE("WarpX::PrintGlobalWarnings");
    const std::string warn_string = m_p_warn_manager->print_global_warnings(when);
    amrex::Print() << warn_string;

}


void 
c_FerroX::ReadData ()
{
#ifdef PRINT_NAME
    amrex::Print() << "\n\n\t\t{************************c_FerroX::ReadData()************************\n";
    amrex::Print() << "\t\tin file: " << __FILE__ << " at line: " << __LINE__ << "\n";
#endif

    m_timestep = 0;
    m_total_steps = 1;
    amrex::ParmParse pp;

    #ifdef TIME_DEPENDENT
        queryWithParser(pp,"timestep", m_timestep);
        queryWithParser(pp,"steps", m_total_steps);
    #endif

    m_pGeometryProperties = std::make_unique<c_GeometryProperties>();

    m_pBoundaryConditions = std::make_unique<c_BoundaryConditions>();
    
#ifdef PRINT_NAME
    amrex::Print() << "\t\t}************************c_FerroX::ReadData()************************\n";
#endif
}


void 
c_FerroX::InitData ()
{
#ifdef PRINT_NAME
    amrex::Print() << "\n\n\t{************************c_FerroX::InitData()************************\n";
    amrex::Print() << "\tin file: " << __FILE__ << " at line: " << __LINE__ << "\n";
#endif
 
    m_pGeometryProperties->InitData();

#ifdef PRINT_NAME
    amrex::Print() << "\t}************************c_FerroX::InitData()************************\n";
#endif
}

AMREX_GPU_MANAGED int FerroX::nsteps;
AMREX_GPU_MANAGED int FerroX::plot_int;

// time step
AMREX_GPU_MANAGED amrex::Real FerroX::dt;

// multimaterial stack geometry
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::DE_lo;
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::FE_lo;
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::SC_lo;
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::DE_hi;
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::FE_hi;
AMREX_GPU_MANAGED amrex::GpuArray<amrex::Real, AMREX_SPACEDIM> FerroX::SC_hi;

// material parameters
AMREX_GPU_MANAGED amrex::Real FerroX::epsilon_0;
AMREX_GPU_MANAGED amrex::Real FerroX::epsilonX_fe;
AMREX_GPU_MANAGED amrex::Real FerroX::epsilonZ_fe;
AMREX_GPU_MANAGED amrex::Real FerroX::epsilon_de;
AMREX_GPU_MANAGED amrex::Real FerroX::epsilon_si;
AMREX_GPU_MANAGED amrex::Real FerroX::alpha; // alpha = 2*alpha_1
AMREX_GPU_MANAGED amrex::Real FerroX::beta; // beta = 4*alpha_11
AMREX_GPU_MANAGED amrex::Real FerroX::gamma; // gamma = 6*alpha_111
AMREX_GPU_MANAGED amrex::Real FerroX::BigGamma;
AMREX_GPU_MANAGED amrex::Real FerroX::g11;
AMREX_GPU_MANAGED amrex::Real FerroX::g44;
AMREX_GPU_MANAGED amrex::Real FerroX::g44_p;
AMREX_GPU_MANAGED amrex::Real FerroX::g12;
AMREX_GPU_MANAGED amrex::Real FerroX::alpha_12;
AMREX_GPU_MANAGED amrex::Real FerroX::alpha_112;
AMREX_GPU_MANAGED amrex::Real FerroX::alpha_123;

// Constants for SC layer calculations
AMREX_GPU_MANAGED amrex::Real FerroX::Nc;
AMREX_GPU_MANAGED amrex::Real FerroX::Nv;
AMREX_GPU_MANAGED amrex::Real FerroX::Ec;
AMREX_GPU_MANAGED amrex::Real FerroX::Ev;
AMREX_GPU_MANAGED amrex::Real FerroX::q;
AMREX_GPU_MANAGED amrex::Real FerroX::kb;
AMREX_GPU_MANAGED amrex::Real FerroX::T;

// P and Phi Bc
AMREX_GPU_MANAGED amrex::Real FerroX::lambda;
AMREX_GPU_MANAGED amrex::GpuArray<int, AMREX_SPACEDIM> FerroX::P_BC_flag_lo;
AMREX_GPU_MANAGED amrex::GpuArray<int, AMREX_SPACEDIM> FerroX::P_BC_flag_hi;

//problem type : initialization of P for 2D/3D/convergence problems
AMREX_GPU_MANAGED int FerroX::prob_type;

AMREX_GPU_MANAGED int FerroX::mlmg_verbosity;

AMREX_GPU_MANAGED int FerroX::TimeIntegratorOrder;

AMREX_GPU_MANAGED amrex::Real FerroX::delta;

void InitializeFerroXNamespace() {

     // ParmParse is way of reading inputs from the inputs file
     // pp.get means we require the inputs file to have it
     // pp.query means we optionally need the inputs file to have it - but we must supply a default here
     ParmParse pp;

     // 0 : P = 0, 1 : dp/dz = p/lambda, 2 : dp/dz = 0
     // 0 : P = 0, 1 : dp/dz = p/lambda, 2 : dp/dz = 0
     amrex::Vector<int> temp_int(AMREX_SPACEDIM);

     if (pp.queryarr("P_BC_flag_lo",temp_int)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             P_BC_flag_lo[i] = temp_int[i];
         }
     }
     if (pp.queryarr("P_BC_flag_hi",temp_int)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             P_BC_flag_hi[i] = temp_int[i];
         }
     }

     inc_step_sign_change = -1;
     pp.query("inc_step_sign_change",inc_step_sign_change);

     pp.get("TimeIntegratorOrder",TimeIntegratorOrder);

     pp.get("prob_type", prob_type);

     mlmg_verbosity = 1;
     pp.query("mlmg_verbosity",mlmg_verbosity);

     // Material Properties

     pp.get("epsilon_0",epsilon_0); // epsilon_0
     pp.get("epsilonX_fe",epsilonX_fe);// epsilon_r for FE
     pp.get("epsilonZ_fe",epsilonZ_fe);// epsilon_r for FE
     pp.get("epsilon_de",epsilon_de);// epsilon_r for DE
     pp.get("epsilon_si",epsilon_si);// epsilon_r for SC
     pp.get("alpha",alpha);
     pp.get("beta",beta);
     pp.get("gamma",FerroX::gamma);
     pp.get("alpha_12",alpha_12);
     pp.get("alpha_112",alpha_112);
     pp.get("alpha_123",alpha_123);
     pp.get("BigGamma",BigGamma);
     pp.get("g11",g11);
     pp.get("g44",g44);
     pp.get("g12",g12);
     pp.get("g44_p",g44_p);

     pp.get("lambda",lambda);

     // Default nsteps to 10, allow us to set it to something else in the inputs file
     nsteps = 10;
     pp.query("nsteps",nsteps);

     // Default plot_int to -1, allow us to set it to something else in the inputs file
     //  If plot_int < 0 then no plot files will be written
     plot_int = -1;
     pp.query("plot_int",plot_int);

     // time step
     pp.get("dt",dt);

     delta = 1.e-6;
     pp.query("delta",delta);

     //stack dimensions in 3D

     amrex::Vector<amrex::Real> temp(AMREX_SPACEDIM);

     if (pp.queryarr("DE_lo",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             DE_lo[i] = temp[i];
         }
     }

     if (pp.queryarr("DE_hi",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             DE_hi[i] = temp[i];
         }
     }

     if (pp.queryarr("FE_lo",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             FE_lo[i] = temp[i];
         }
     }

     if (pp.queryarr("FE_hi",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             FE_hi[i] = temp[i];
         }
     }

     if (pp.queryarr("SC_lo",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             SC_lo[i] = temp[i];
         }
     }

     if (pp.queryarr("SC_hi",temp)) {
         for (int i=0; i<AMREX_SPACEDIM; ++i) {
             SC_hi[i] = temp[i];
         }
     }

     // For Silicon:
     // Nc = 2.8e25 m^-3
     // Nv = 1.04e25 m^-3
     // Band gap Eg = 1.12eV
     // 1eV = 1.602e-19 J

     Nc = 2.8e25;
     Nv = 1.04e25;
     Ec = 0.56;
     Ev = -0.56;
     q = 1.602e-19;
     kb = 1.38e-23; // Boltzmann constant
     T = 300; // Room Temp

}


