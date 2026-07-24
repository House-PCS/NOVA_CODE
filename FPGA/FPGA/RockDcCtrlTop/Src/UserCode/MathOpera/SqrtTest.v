module SqrtTest #(
    parameter d_width = 32,
    q_width = d_width / 2,
    r_width = q_width + 1
)
(

    input clk,
    input rst,
    input [d_width-1:0] D,
    output reg [q_width-1:0] Q,
    output reg [r_width-1:0] R,
    input ivalid,
    output reg ovalid

);

    reg        [d_width-1:0] D_t     [q_width:1];
    reg        [q_width-1:0] Q_t     [q_width:1];
    reg signed [r_width-1:0] R_t     [q_width:1];
    reg                      ivalid_t[q_width:1];

    always @(posedge clk) begin
        if (rst) begin
            R_t[q_width]      <= {r_width{1'b0}};
            D_t[q_width]      <= {d_width{1'b0}};
            Q_t[q_width]      <= {q_width{1'b0}};
            ivalid_t[q_width] <= 1'b0;
        end else begin
            if (ivalid) begin
                R_t[q_width]<={R[r_width-3:0],D[d_width-1:d_width-2]} - {{q_width-1{1'b0}},2'b01};
                D_t[q_width] <= D;
                Q_t[q_width] <= {q_width{1'b0}};
                ivalid_t[q_width] <= 1'b1;
            end else begin
                R_t[q_width]      <= {r_width{1'b0}};
                D_t[q_width]      <= {d_width{1'b0}};
                Q_t[q_width]      <= {q_width{1'b0}};
                ivalid_t[q_width] <= 1'b0;
            end
        end
    end

    generate
        genvar i;
        for (i = q_width - 1; i >= 1; i = i - 1) begin : U
            always @(posedge clk) begin
                if (rst) begin
                    Q_t[i]      <= {q_width{1'b0}};
                    R_t[i]      <= {r_width{1'b0}};
                    D_t[i]      <= {d_width{1'b0}};
                    ivalid_t[i] <= 1'b0;
                end else begin
                    if (ivalid_t[i+1]) begin
                        if (R_t[i+1] >= 0) begin
                            Q_t[i] <= {Q_t[i+1][q_width-2:0], 1'b1};
                            R_t[i]<={R_t[i+1][r_width-3:0],D_t[i+1][2*i-1:2*i-2]} - {1'b0,Q_t[i+1][q_width-4:0],1'b1,2'b01};
                            D_t[i] <= D_t[i+1];
                            ivalid_t[i] <= 1'b1;
                        end else begin
                            Q_t[i] <= {Q_t[i+1][q_width-2:0], 1'b0};
                            R_t[i]<={R_t[i+1][r_width-3:0],D_t[i+1][2*i-1:2*i-2]} + {1'b0,Q_t[i+1][q_width-4:0],1'b0,2'b11};
                            D_t[i] <= D_t[i+1];
                            ivalid_t[i] <= 1'b1;
                        end
                    end else begin
                        Q_t[i]      <= {q_width{1'b0}};
                        R_t[i]      <= {r_width{1'b0}};
                        D_t[i]      <= {d_width{1'b0}};
                        ivalid_t[i] <= 1'b0;
                    end
                end
            end
        end
    endgenerate

    always @(posedge clk) begin
        if (rst) begin
            Q      <= {q_width{1'b0}};
            R      <= {r_width{1'b0}};
            ovalid <= 1'b0;
        end else begin
            if (ivalid_t[1]) begin
                if (R_t[1] >= 0) begin
                    Q <= {Q_t[1][q_width-2:0], 1'b1};
                    R <= R_t[1];
                end else begin
                    Q <= {Q_t[1][q_width-2:0], 1'b0};
                    R <= R_t[1] + {1'b0, Q_t[1][q_width-3:0], 1'b0, 1'b1};
                end
                ovalid <= 1'b1;
            end else begin
                Q      <= {q_width{1'b0}};
                R      <= {r_width{1'b0}};
                ovalid <= 1'b0;
            end
        end
    end

endmodule
