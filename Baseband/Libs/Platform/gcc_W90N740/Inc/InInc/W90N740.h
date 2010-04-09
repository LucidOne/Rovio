#ifndef  _w90n740_h_
#define  _w90n740_h_


// ASIC Address Definition
#define VPint   *(volatile unsigned int *)
#define VPshort *(volatile unsigned short *)
#define VPchar  *(volatile unsigned char *)


#define Base_Addr 0xFFF00000


// Advanced Interrupt Controller Registers
#define AIC_SCR(n)		(VPint(Base_Addr+0x00082000+(n)*4))
#define AIC_SCR_nIRQ0	(VPint(Base_Addr+0x00082008))
#define AIC_SCR_nIRQ1	(VPint(Base_Addr+0x0008200c))
#define AIC_SCR_nIRQ2	(VPint(Base_Addr+0x00082010))
#define AIC_SCR_nIRQ3	(VPint(Base_Addr+0x00082014))
#define AIC_SCR_TINT0	(VPint(Base_Addr+0x0008201C))
#define AIC_SCR_EMC0   (VPint(Base_Addr+0x0008202c))
#define AIC_SCR_EMC1   (VPint(Base_Addr+0x00082030))
#define AIC_SCR_EMCTX0 (VPint(Base_Addr+0x00082034))
#define AIC_SCR_EMCTX1 (VPint(Base_Addr+0x00082038))
#define AIC_SCR_EMCRX0 (VPint(Base_Addr+0x0008203c))
#define AIC_SCR_EMCRX1 (VPint(Base_Addr+0x00082040))
#define AIC_IRSR       (VPint(Base_Addr+0x00082100))
#define AIC_IASR       (VPint(Base_Addr+0x00082104))
#define AIC_ISR        (VPint(Base_Addr+0x00082108))
#define AIC_IPER       (VPint(Base_Addr+0x0008210c))
#define AIC_ISNR       (VPint(Base_Addr+0x00082110))
#define AIC_IMR        (VPint(Base_Addr+0x00082114))
#define AIC_OISR       (VPint(Base_Addr+0x00082118))
#define AIC_MECR       (VPint(Base_Addr+0x00082120))
#define AIC_MDCR       (VPint(Base_Addr+0x00082124))
#define AIC_SSCR       (VPint(Base_Addr+0x00082128))
#define AIC_SCCR       (VPint(Base_Addr+0x0008212c))
#define AIC_EOSCR      (VPint(Base_Addr+0x00082130))

// MAC Interrupt Sources
#define WDTINT	1
#define nIRQ0	2
#define nIRQ1	3
#define nIRQ2	4
#define nIRQ3	5
#define UARTINT	6
#define TINT0	7
#define TINT1	8
#define	USBINT0	9
#define	USBINT1	10
#define EMCTXINT0 13
#define EMCTXINT1 14
#define EMCRXINT0 15
#define EMCRXINT1 16
#define GDMAINT0	17
#define GDMAINT1	18



//TIMER Control Registers
#define TCR_0	(VPint(Base_Addr+0x00081000))
#define TCR_1	(VPint(Base_Addr+0x00081004))
#define TICR_0	(VPint(Base_Addr+0x00081008))
#define TICR_1	(VPint(Base_Addr+0x0008100C))
#define TDR_0	(VPint(Base_Addr+0x00081010))
#define TDR_1	(VPint(Base_Addr+0x00081014))
#define TISR	(VPint(Base_Addr+0x00081018))
#define WTCR	(VPint(Base_Addr+0x0008101c))


// EMC 0 Control Registers
#define CAMCMR_0       (VPint(Base_Addr+0x3000))     // CAM Registers
#define CAMEN_0        (VPint(Base_Addr+0x3004))
#define CAM0M_Base_0   Base_Addr+0x3008
#define CAM0L_Base_0   Base_Addr+0x300c
#define CAMxM_Reg_0(x) (VPint(CAM0M_Base_0+x*0x8))
#define CAMxL_Reg_0(x) (VPint(CAM0L_Base_0+x*0x8))
#define MIEN_0         (VPint(Base_Addr+0x3088))     // MAC Registers
#define MCMDR_0        (VPint(Base_Addr+0x308c))
#define MIID_0         (VPint(Base_Addr+0x3090))
#define MIIDA_0        (VPint(Base_Addr+0x3094))
#define MPCNT_0        (VPint(Base_Addr+0x3098))
#define TXDLSA_0       (VPint(Base_Addr+0x309c))     // DMA Registers
#define RXDLSA_0       (VPint(Base_Addr+0x30a0))
#define DMARFC_0       (VPint(Base_Addr+0x30a4))
#define TSDR_0         (VPint(Base_Addr+0x30a8))
#define RSDR_0         (VPint(Base_Addr+0x30ac))
#define FIFOTHD_0      (VPint(Base_Addr+0x30b0))     // Test Registers
// EMC 0 Status Registers
#define MISTA_0        (VPint(Base_Addr+0x30b4))     // MAC Registers
#define MGSTA_0        (VPint(Base_Addr+0x30b8))
#define MRPC_0         (VPint(Base_Addr+0x30bc))
#define MRPCC_0        (VPint(Base_Addr+0x30c0))
#define MREPC_0        (VPint(Base_Addr+0x30c4))
#define DMARFS_0       (VPint(Base_Addr+0x30c8))     // DMA Registers
#define CTXDSA_0       (VPint(Base_Addr+0x30cc))
#define CTXBSA_0       (VPint(Base_Addr+0x30d0))
#define CRXDSA_0       (VPint(Base_Addr+0x30d4))
#define CRXBSA_0       (VPint(Base_Addr+0x30d8))
// EMC 0 Diagnostic Registers
#define TICS_0         (VPint(Base_Addr+0x3100))     // Test Registers
#define RXFSM_0        (VPint(Base_Addr+0x3200))     // Debug Registers
#define TXFSM_0        (VPint(Base_Addr+0x3204))
#define FSM0_0         (VPint(Base_Addr+0x3208))
#define FSM1_0         (VPint(Base_Addr+0x320c))
#define DCR_0          (VPint(Base_Addr+0x3210))
#define BISTR_0        (VPint(Base_Addr+0x3300))     // BIST Registers


// EMC 1 Control Registers
#define CAMCMR_1       (VPint(Base_Addr+0x3800))     // CAM Registers
#define CAMEN_1        (VPint(Base_Addr+0x3804))
#define CAM0M_Base_1   Base_Addr+0x3808
#define CAM0L_Base_1   Base_Addr+0x380c
#define CAMxM_Reg_1(x) (VPint(CAM0M_Base_1+x*0x8))
#define CAMxL_Reg_1(x) (VPint(CAM0L_Base_1+x*0x8))
#define MIEN_1         (VPint(Base_Addr+0x3888))     // MAC Registers
#define MCMDR_1        (VPint(Base_Addr+0x388c))
#define MIID_1         (VPint(Base_Addr+0x3890))
#define MIIDA_1        (VPint(Base_Addr+0x3894))
#define MPCNT_1        (VPint(Base_Addr+0x3898))
#define TXDLSA_1       (VPint(Base_Addr+0x389c))     // DMA Registers
#define RXDLSA_1       (VPint(Base_Addr+0x38a0))
#define DMARFC_1       (VPint(Base_Addr+0x38a4))
#define TSDR_1         (VPint(Base_Addr+0x38a8))
#define RSDR_1         (VPint(Base_Addr+0x38ac))
#define FIFOTHD_1      (VPint(Base_Addr+0x38b0))     // Test Registers
// EMC 1 Status Registers
#define MISTA_1        (VPint(Base_Addr+0x38b4))     // MAC Registers
#define MGSTA_1        (VPint(Base_Addr+0x38b8))
#define MRPC_1         (VPint(Base_Addr+0x38bc))
#define MRPCC_1        (VPint(Base_Addr+0x38c0))
#define MREPC_1        (VPint(Base_Addr+0x38c4))
#define DMARFS_1       (VPint(Base_Addr+0x38c8))     // DMA Registers
#define CTXDSA_1       (VPint(Base_Addr+0x38cc))
#define CTXBSA_1       (VPint(Base_Addr+0x38d0))
#define CRXDSA_1       (VPint(Base_Addr+0x38d4))
#define CRXBSA_1       (VPint(Base_Addr+0x38d8))
// EMC 1 Diagnostic Registers
#define TICS_1         (VPint(Base_Addr+0x3900))     // Test Registers
#define RXFSM_1        (VPint(Base_Addr+0x3a00))     // Debug Registers
#define TXFSM_1        (VPint(Base_Addr+0x3a04))
#define FSM0_1         (VPint(Base_Addr+0x3a08))
#define FSM1_1         (VPint(Base_Addr+0x3a0c))
#define BISTR_1        (VPint(Base_Addr+0x3b00))     // BIST Registers


// CAM Command Register(CAMCMR)
#define CAM_AUP  0x0001  // Accept Packets with Unicast Address
#define CAM_AMP  0x0002  // Accept Packets with Multicast Address
#define CAM_ABP  0x0004  // Accept Packets with Broadcast Address
#define CAM_CCAM 0x0008  // 0: Accept Packets CAM Recognizes and Reject Others
                         // 1: Reject Packets CAM Recognizes and Accept Others
#define CAM_ECMP 0x0010  // Enable CAM Compare


// MAC Interrupt Enable Register(MIEN)
#define EnRXINTR 0x00000001  // Enable Interrupt on Receive Interrupt
#define EnCRCE   0x00000002  // Enable CRC Error Interrupt
#define EnRXOV   0x00000004  // Enable Receive FIFO Overflow Interrupt
#define EnPTLE   0x00000008  // Enable Packet Too Long Interrupt
#define EnRXGD   0x00000010  // Enable Receive Good Interrupt
#define EnALIE   0x00000020  // Enable Alignment Error Interrupt
#define EnRP     0x00000040  // Enable Runt Packet on Receive Interrupt
#define EnMMP    0x00000080  // Enable More Missed Packets Interrupt
#define EnDFO    0x00000100  // Enable DMA receive frame over maximum size Interrupt
#define EnDEN    0x00000200  // Enable DMA early notification Interrupt
#define EnRDU    0x00000400  // Enable Receive Descriptor Unavailable Interrupt
#define EnRxBErr 0x00000800  // Enable Receive Bus ERROR interrupt
#define EnNATOK  0x00001000  // Enable NAT Processing OK Interrupt
#define EnNATErr 0x00002000  // Enable NAT Processing Error Interrupt
#define EnCFR    0x00004000  // Enable Control Frame Receive Interrupt
#define EnTXINTR 0x00010000  // Enable Interrupt on Transmit Interrupt
#define EnTXEMP  0x00020000  // Enable Transmit FIFO Empty Interrupt
#define EnTXCP   0x00040000  // Enable Transmit Completion Interrupt
#define EnEXDEF  0x00080000  // Enable Defer Interrupt
#define EnNCS    0x00100000  // Enable No Carrier Sense Interrupt
#define EnTXABT  0x00200000  // Enable Transmit Abort Interrupt
#define EnLC     0x00400000  // Enable Late Collision Interrupt
#define EnTDU    0x00800000  // Enable Transmit Descriptor Unavailable Interrupt
#define EnTxBErr 0x01000000  // Enable Transmit Bus ERROR Interrupt


// MAC Command Register(MCMDR)
#define MCMDR_RXON    0x00000001  // Receive ON
#define MCMDR_ALP     0x00000002  // Accept Long Packet
#define MCMDR_ARP     0x00000004  // Accept Runt Packet
#define MCMDR_ACP     0x00000008  // Accept Control Packet
#define MCMDR_AEP     0x00000010  // Accept Error Packet
#define MCMDR_SPCRC   0x00000020  // Accept Strip CRC Value
#define MCMDR_TXON    0x00000100  // Transmit On
#define MCMDR_NDEF    0x00000200  // No defer
#define MCMDR_SDPZ    0x00010000  // Send Pause
#define MCMDR_EnSQE   0x00020000  // Enable SQE test
#define MCMDR_FDUP    0x00040000  // Full Duplex
#define MCMDR_EnMDC   0x00080000  // Enable MDC signal
#define MCMDR_OPMOD   0x00100000  // Operation Mode
#define MCMDR_LBK     0x00200000  // Loop Back
#define MCMDR_EnRMII  0x00400000  // Enable RMII
#define MCMDR_LPCS    0x00800000  // Low Pin Count Package Switch
#define MCMDR_LAN     0x01000000  // LAN Port Setting Mode
#define MCMDR_EnTMode 0x02000000  // Enable Test Mode


// MAC MII Management Data Control and Address Register(MIIDA)
//#define MDCCR    0x00000000  // MDC clock rating
#define MDCCR    0x00380000  // MDC clock rating
#define PHYAD    0x00000100  // PHY Address
#define PHYWR    0x00010000  // Write Operation
#define PHYBUSY  0x00020000  // Busy Bit
#define PHYPreSP 0x00040000  // Preamble Suppress


// FIFO Threshold Adjustment Register(FIFOTHD)
#define SWR        0x00010000  // Software Reset
#define TxTHD_1    0x00000100  // 1/4 Transmit FIFO Threshold
#define TxTHD_2    0x00000200  // 2/4 Transmit FIFO Threshold
#define TxTHD_3    0x00000300  // 3/4 Transmit FIFO Threshold
#define RxTHD_1    0x00000001  // 1/4 Receive FIFO Threshold
#define RxTHD_2    0x00000002  // 2/4 Receive FIFO Threshold
#define RxTHD_3    0x00000003  // 3/4 Receive FIFO Threshold
#define Blength_8  0x00100000  // DMA burst length 8 beats
#define Blength_12 0x02000000  // DMA burst length 12 beats
#define Blength_16 0x03000000  // DMA burst length 16 beats


// MAC Interrupt Status Register(MISTA)
#define MISTA_RXINTR 0x00000001  // Interrupt on Receive
#define MISTA_CRCE   0x00000002  // CRC Error
#define MISTA_RXOV   0x00000004  // Receive FIFO Overflow error
#define MISTA_PTLE   0x00000008  // Packet Too Long Error
#define MISTA_RXGD   0x00000010  // Receive Good
#define MISTA_ALIE   0x00000020  // Alignment Error
#define MISTA_RP     0x00000040  // Runt Packet
#define MISTA_MMP    0x00000080  // More Missed Packets than miss rolling over counter flag
#define MISTA_DFOI   0x00000100  // DMA receive frame over maximum size interrupt
#define MISTA_DENI   0x00000200  // DMA early notification interrupt
#define MISTA_RDU    0x00000400  // Receive Descriptor Unavailable interrupt
#define MISTA_RxBErr 0x00000800  // Receive Bus Error interrupt
#define MISTA_NATOK  0x00001000  // NAT Processing OK
#define MISTA_NATErr 0x00002000  // NAT Processing Error
#define MISTA_CFR    0x00004000  // Control Frame Receive
#define MISTA_TXINTR 0x00010000  // Interrupt on Transmit
#define MISTA_TXEMP  0x00020000  // Transmit FIFO Empty
#define MISTA_TXCP   0x00040000  // Transmit Completion
#define MISTA_EXDEF  0x00080000  // Defer
#define MISTA_NCS    0x00100000  // No Carrier Sense
#define MISTA_TXABT  0x00200000  // Transmit Abort
#define MISTA_LC     0x00400000  // Late Collision
#define MISTA_TDU    0x00800000  // Transmit Descriptor Unavailable interrupt
#define MISTA_TxBErr 0x01000000  // Transmit Bus Error interrupt


// MAC General Status Register(MGSTA)
#define MGSTA_CFR  0x00000001  // Control Frame Received
#define MGSTA_RXHA 0x00000002  // Reception Halted
#define MGSTA_DEF  0x00000010  // Deferred transmission
#define MGSTA_PAU  0x00000020  // Pause Bit
#define MGSTA_SQE  0x00000040  // Signal Quality Error
#define MGSTA_TXHA 0x00000080  // Transmission Halted


// PHY Register Description
#define PHY_CNTL_REG    0x00
#define PHY_STATUS_REG  0x01
#define PHY_ID1_REG     0x02
#define PHY_ID2_REG     0x03
#define PHY_ANA_REG     0x04
#define PHY_ANLPA_REG   0x05
#define PHY_ANE_REG     0x06

#define PHY_DSC_REG     0x10
#define PHY_DSCS_REG    0x11
#define PHY_10BTCS_REG  0x12
#define PHY_SINT_REG    0x15
#define PHY_SREC_REG    0x16
#define PHY_DISC_REG    0x17

//PHY Control Register
#define RESET_PHY       1 << 15
#define ENABLE_LOOPBACK 1 << 14
#define DR_100MB        1 << 13
#define ENABLE_AN       1 << 12
#define PHY_POWER_DOWN  1 << 11
#define PHY_MAC_ISOLATE 1 << 10
#define RESTART_AN      1 << 9
#define PHY_FULLDUPLEX  1 << 8
#define PHY_COL_TEST    1 << 7

// PHY Status Register
#define AN_COMPLETE     1 << 5

// PHY Auto-negotiation Advertisement Register
#define DR100_TX_FULL   1 << 8
#define DR100_TX_HALF   1 << 7
#define DR10_TX_FULL    1 << 6
#define DR10_TX_HALF    1 << 5


#define MAC_ADDR_SIZE         6

#define MaxRxFrameSize        1520 // Rx Frame Max Size = 1520

#define MaxRxFrameDescriptors 32   // Max number of Rx Frame Descriptors
#define MaxTxFrameDescriptors 16   // Max number of Tx Frame Descriptors


// RX Frame Descriptor's Owner bit
#define RXfOwnership_DMA 0x80000000  // 10 = DMA
#define RXfOwnership_NAT 0xc0000000  // 11 = NAT
#define RXfOwnership_CPU 0x3fffffff  // 00 = CPU

// TX Frame Descriptor's Owner bit
#define TXfOwnership_DMA 0x80000000  // 1 = DMA
#define TXfOwnership_CPU 0x7fffffff  // 0 = CPU


// Rx Frame Descriptor Status
#define RXFD_Hit     0x2000  // current packet is hit with NAT entry table
#define RXFD_IPHit   0x1000  // current packet is hit on IP address
#define RXFD_PortHit 0x0800  // current packet is hit on Port Number
#define RXFD_Inverse 0x0400  // current hit entry is setting on inverse mode
#define RXFD_NATFSH  0x0200  // NAT Processing Finish
#define RXFD_RP      0x0040  // Runt Packet
#define RXFD_ALIE    0x0020  // Alignment Error
#define RXFD_RXGD    0x0010  // Receiving Good packet received
#define RXFD_PTLE    0x0008  // Packet Too Long Error
#define RXFD_CRCE    0x0002  // CRC Error
#define RXFD_RXINTR  0x0001  // Interrupt on receive

// Rx Frame Descriptor NAT Information
#define RXFD_NAT_URG     0x20000000  // TCP status(URG)
#define RXFD_NAT_ACK     0x10000000  // TCP status(ACK)
#define RXFD_NAT_PSH     0x08000000  // TCP status(PSH)
#define RXFD_NAT_RST     0x04000000  // TCP status(RST)
#define RXFD_NAT_SYN     0x02000000  // TCP status(SYN)
#define RXFD_NAT_FIN     0x01000000  // TCP status(FIN)
#define RXFD_NAT_UCKERR  0x00400000  // TCP/UCKS Error
#define RXFD_NAT_TUERR   0x00200000  // TCP/UDP Error
#define RXFD_NAT_NHERR   0x00100000  // No Hit Error
#define RXFD_NAT_PPPCaps 0x00000040  // PPPoE datagram encapsulated
#define RXFD_NAT_PPPoE   0x00000020  // PPPoE protocol
#define RXFD_NAT_UCKS    0x00000010  // UDP protocol with skip checksum replacement
#define RXFD_NAT_UDP     0x00000008  // apply UDP protocol
#define RXFD_NAT_TCP     0x00000004  // apply TCP protocol
#define RXFD_NAT_LAN     0x00000002  // internal (LAN) port gets hit
#define RXFD_NAT_Hit     0x00000001  // current packet is hit with NAT entry table


// Tx Frame Descriptor's Control bits
#define MACTxIntEn    0x04
#define CRCMode       0x02
#define NoCRCMode     0x00
#define PaddingMode   0x01
#define NoPaddingMode 0x00

// Tx Frame Descriptor Status
#define TXFD_TXINTR 0x0001  // Interrupt on Transmit
#define TXFD_DEF    0x0002  // Transmit deferred
#define TXFD_TXCP   0x0008  // Transmission Completion
#define TXFD_EXDEF  0x0010  // Exceed Deferral
#define TXFD_NCS    0x0020  // No Carrier Sense Error
#define TXFD_TXABT  0x0040  // Transmission Abort
#define TXFD_LC     0x0080  // Late Collision
#define TXFD_TXHA   0x0100  // Transmission halted
#define TXFD_PAU    0x0200  // Paused
#define TXFD_SQE    0x0400  // SQE error



#endif  /* _w90n740_h_ */
